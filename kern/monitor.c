// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", "Display this list of commands", mon_help },
	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
	{ "backtrace", "Display stack backtrace", mon_backtrace },
	{ "showmappings", "Display physical page mappings", mon_showmappings},
	{ "setpermbits", "Set, clear, or change the permissions of any mapping in the current address space", mon_setpermbits},
	{ "dumpmem", "Dump the contents of a range of memory given either a virtual or physical address range", mon_dumpmem},
};

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(commands); i++)
		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
	cprintf("Stack backtrace:\n");
	uint32_t cur_ebp, cur_eip;
	struct Eipdebuginfo info;

	cur_ebp = read_ebp();
	while (cur_ebp) {
		cprintf("  ebp %08x  eip %08x  args %08x %08x %08x %08x %08x\n",
			cur_ebp, ((uint32_t *)cur_ebp)[1], ((uint32_t *)cur_ebp)[2], ((uint32_t *)cur_ebp)[3],
			((uint32_t *)cur_ebp)[4], ((uint32_t *)cur_ebp)[5], ((uint32_t *)cur_ebp)[6], ((uint32_t *)cur_ebp)[7]);

		cur_eip = ((uint32_t *)cur_ebp)[1];
		debuginfo_eip(cur_eip, &info);
		cprintf("         %s:%d: %.*s+%d\n", info.eip_file, info.eip_line, info.eip_fn_namelen, info.eip_fn_name, cur_eip - info.eip_fn_addr);

		cur_ebp = ((uint32_t *)cur_ebp)[0];
	}
	return 0;
}

int
mon_showmappings(int argc, char **argv, struct Trapframe *tf)
{
	if (argc != 3 && argc != 2) {
		cprintf("Wrong number of arguments.\n");
		cprintf("Usage: showmappings begin_vaddr [end_vaddr]\n");
		return 1;
	}

	int begin, end;
	struct PageInfo *pp;
	pte_t *pptr;
	char buf[15];

	begin = ROUNDDOWN((uint32_t)strtol(argv[1], NULL, 16), PGSIZE);
	if (argc == 2)
		end = begin;
	else
		end = ROUNDDOWN((uint32_t)strtol(argv[2], NULL, 16), PGSIZE);

	if (begin > 0xf7ffffff || end > 0xf7ffffff) {
		cprintf("Virtual Address out of range.\n");
		return 1;
	}

	cprintf("%8s\t%8s\t%12s\n", "VA", "PA", "PERMBITS");
	for (; begin <= end; begin += PGSIZE) {
		pp = page_lookup(kern_pgdir, (void *)begin, &pptr);
		if (pp != NULL) {
			pageperm2str(*pptr, buf);
			cprintf("%08x\t%08x\t%12s\n", begin, page2pa(pp), buf);
		}
		else
			cprintf("%08x\t%8s\t%12s\n", begin, "none", "none", "none");
	}
	return 0;
}

int
mon_setpermbits(int argc, char **argv, struct Trapframe *tf) {
	if (argc != 3) {
		cprintf("Wrong number of arguments.\n");
		cprintf("Usage: setpermbits +/-permbits vaddr\n");
		return 1;
	}

	int begin;
	struct PageInfo *pp;
	pte_t *pptr, pte;
	char buf_old[15], buf_new[15];

   	begin = ROUNDDOWN((uint32_t) strtol(argv[2], NULL, 0), PGSIZE);

	if (begin > 0xf7ffffff) {
		cprintf("Virtual Address out of range.\n");
		return 1;
	}

	pp = page_lookup(kern_pgdir, (void *)begin, &pptr);

	if (pp == NULL) {
		cprintf("No mpping exists.\n");
		return 1;
	}

	pte = *pptr;
	if (*argv[1] == '+') 
		*pptr |= str2pageperm(argv[1] + 1);
	else if (*argv[1] == '-')
		*pptr &= ~str2pageperm(argv[1] + 1);

	cprintf("Virtual\t\tPhysical\tOld Priority\tNew Priority\t\n");
	cprintf("%08x\t%08x\t%12s\t%12s\n", begin, page2pa(pp), pageperm2str(pte, buf_old), pageperm2str(*pptr, buf_new));

	return 0;
}

int mon_dumpmem(int argc, char **argv, struct Trapframe *tf) {
	if (argc != 4) {
		cprintf("Wrong number of arguments.\n");
		cprintf("Usage: dumpmem -p/v begin_addr end_addr\n");
		return 1;
	}

	int begin, end;

   	begin = (uint32_t) strtol(argv[2], NULL, 0);
   	end = (uint32_t) strtol(argv[3], NULL, 0);

	if (argv[1][1] == 'p') {
		if ((PGNUM(begin) >= npages) || (PGNUM(end) >= npages)) {
            cprintf("Physical address out of range.\n");
            return 1;
        }
        begin = (uint32_t) KADDR(begin);
        end = (uint32_t) KADDR(end);
	}

	if (begin >= 0xf7ffffff || end >= 0xf7ffffff) {
		cprintf("Virtual address out of range.\n");
		return 1;
	}

	cprintf("Virtual\t\tPhysical\tMemory Contents\n");
    while (begin <= end) {
        int i;
        pte_t *pptr;

        cprintf("%08x\t", begin);
        if (page_lookup(kern_pgdir, (void *) begin, &pptr) == NULL || *pptr == 0) {
            cprintf("No Mapping\n");
            begin += PGSIZE - begin % PGSIZE;
            continue;
        }

        cprintf("%08x\t", PTE_ADDR(*pptr) | PGOFF(begin));

        for (i = 0; i < 16; i++, begin++)
            cprintf("%02x ", *(unsigned char *) begin);

        cprintf("\n");
    }

	return 0;
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))
			*buf++ = 0;
		if (*buf == 0)
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
