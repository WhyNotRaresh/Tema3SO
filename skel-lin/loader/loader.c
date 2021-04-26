/*
 * Loader Implementation
 *
 * 2018, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>

#include "exec_parser.h"


#define PAGE_SIZE getpagesize()


static so_exec_t *exec;
static int exec_fd;
static void* prev_handler;


void alloc_seg_data(so_seg_t *segment)
{
	int possible_pages = segment->file_size / PAGE_SIZE;
	segment->data = (void*) malloc(possible_pages * sizeof(char));
	memset(segment->data, 0, possible_pages * sizeof(char));
}


void so_map_page(uintptr_t page_fault_addr, so_seg_t *segment)
{
	if (segment->data == NULL)
		alloc_seg_data(segment);

	/* Getting page start address */

	int page_index = (page_fault_addr - segment->vaddr) / PAGE_SIZE;
	uintptr_t page_addr = segment->vaddr + page_index * PAGE_SIZE;

	if (((char*)(segment->data))[page_index] == 1) {
		fprintf(stderr, "Invalid permissions\n");
		signal(SIGSEGV, prev_handler);
		raise(SIGSEGV);
	}

	/* Mapping page */

	char* mapped_addr = mmap((int*) page_addr, PAGE_SIZE, PROT_WRITE,
		MAP_SHARED | MAP_FIXED | MAP_ANON, -1, 0);

	if (mapped_addr == MAP_FAILED){
		fprintf(stderr, "Error mapping page.\n\
			error numner: %d\n", errno);
		exit(EXIT_FAILURE);
	}

	((char*)(segment->data))[page_index] = 1;

	/* Copying from file into memory */

	lseek(exec_fd, segment->offset + page_index * PAGE_SIZE, SEEK_SET);
	int bytes_read = read(exec_fd, (void*) mapped_addr, PAGE_SIZE);
	if (bytes_read == -1){
		fprintf(stderr, "Error reading from file\n");
		exit(EXIT_FAILURE);
	}

	/* Last page */

	if (page_index == segment->file_size / PAGE_SIZE) {
		//fprintf(stderr, "%s\n", "CUUC1");
		memset((void*)((int)(segment->vaddr) + segment->file_size),
			0, segment->mem_size - segment->file_size);
		//fprintf(stderr, "%s\n", "CUUC2");
	}

	/* Setting page permissions */

	mprotect(mapped_addr, PAGE_SIZE, PROT_NONE);

	int perm_flags = 0;
	if ((segment->perm & PERM_R) != 0) perm_flags |= PROT_READ;
	if ((segment->perm & PERM_W) != 0) perm_flags |= PROT_WRITE;
	if ((segment->perm & PERM_X) != 0) perm_flags |= PROT_EXEC;

	mprotect(mapped_addr, PAGE_SIZE, perm_flags);
}

void so_sigaction(int sig_no, siginfo_t* sig_info, void* context)
{
	if (sig_info == NULL) exit(EXIT_FAILURE);
	uintptr_t page_fault_addr = (int)sig_info->si_addr;

	int i;
	for (i = 0; i < exec->segments_no; i++) {
		so_seg_t *segment = (exec->segments) + i;

		/* Checking boundaries of this segment */

		if (segment->vaddr <= page_fault_addr &&
			page_fault_addr < segment->vaddr + segment->mem_size) {
			so_map_page(page_fault_addr, segment);
			return;
		}
	}

	fprintf(stderr, "Seg Fault outside of Exec segments\n");
	signal(SIGSEGV, prev_handler);
	raise(SIGSEGV);
}

int so_init_loader(void)
{
	// Remembering default handler
	prev_handler = signal(SIGSEGV, NULL);

	// Loading our own handler
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));
	sa.sa_sigaction = so_sigaction;
	sigaction(SIGSEGV, &sa, NULL);

	exec = (so_exec_t*) calloc(1, sizeof(so_exec_t));

	return 0;
}

int so_execute(char *path, char *argv[])
{
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	exec_fd = open(path, O_RDONLY, 0644);
	so_start_exec(exec, argv);

	return -1;
}
