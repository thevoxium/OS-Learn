#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define PHYSICAL_MEMORY (10000)
#define MAX_PROCESS 10

typedef struct MMU {
  int base;
  int bound;
} MMU;

typedef struct PCB {
  int pid;
  int is_used;
  int base;
  int bound;
} PCB;

MMU cpu_mmu;
PCB process_table[MAX_PROCESS];
char physical_memory[PHYSICAL_MEMORY];
PCB *current_process = NULL;

void os_init(void);
int create_process(int size);
void context_switch(int next_pid);

int allocate_memory(int size);

int translate(int virtual_address);

void print_mmu_state(void);
void print_process_table(void);
void print_mem_map(void);

int main() {
  os_init();
  printf("System initialized.\n");
  print_mem_map();

  int pid1 = create_process(500);
  int pid2 = create_process(1000);
  int pid3 = create_process(200);

  printf("\nAfter process creation:\n");
  print_process_table();
  print_mem_map();

  printf("\n--- Starting Simulation ---\n");

  printf("\n[OS]: Switching to Process %d\n", pid1);
  context_switch(pid1);
  print_mmu_state();

  printf("[Process %d]: Accessing VA: 0 -> PA: %d\n", pid1, translate(0));
  printf("[Process %d]: Accessing VA: 499 -> PA: %d\n", pid1, translate(499));
  printf("[Process %d]: Accessing VA: 500 -> ", pid1); // This should fault!
  translate(500);

  printf("\n[OS]: Switching to Process %d\n", pid2);
  context_switch(pid2);
  print_mmu_state();

  printf("[Process %d]: Accessing VA: 0 -> PA: %d\n", pid2, translate(0));
  printf("[Process %d]: Accessing VA: 249 -> PA: %d\n", pid2, translate(249));

  return 0;
}

void os_init(void) {
  for (int i = 0; i < PHYSICAL_MEMORY; i++) {
    physical_memory[i] = '.';
  }

  for (int i = 0; i < MAX_PROCESS; i++) {
    PCB *process = &process_table[i];
    process->is_used = 0;
    process->base = 0;
    process->bound = 0;
    process->pid = 0;
  }
  cpu_mmu.base = 0;
  cpu_mmu.bound = 0;
  current_process = NULL;
}

int allocate_memory(int size) {
  int free_block_start = -1;
  int current_block_size = 0;

  for (int i = 0; i < PHYSICAL_MEMORY; i++) {
    if (physical_memory[i] == '.') {
      if (current_block_size == 0) {
        free_block_start = i;
      }
      current_block_size++;
      if (current_block_size == size) {
        for (int j = free_block_start; j < free_block_start + size; j++) {
          physical_memory[j] = 'A';
        }
        return free_block_start;
      }
    } else {
      current_block_size = 0;
      free_block_start = -1;
    }
  }
  return -1;
}

int create_process(int size) {
  int base_memory = allocate_memory(size);
  if (base_memory == -1) {
    fprintf(stderr, "memory allocation for the process failed!");
    return -1;
  }
  int free_pcb_slot = -1;
  int pid;
  for (int i = 0; i < MAX_PROCESS; i++) {
    PCB *process = &process_table[i];
    if (process->is_used == 0) {
      free_pcb_slot = i;
      process->pid = i;
      process->base = base_memory;
      process->is_used = 1;
      process->bound = size;
      pid = i;
      break;
    }
  }

  if (free_pcb_slot == -1) {
    fprintf(stderr, "No free slot found in Process table");
    return -1;
  }

  return pid;
}

void context_switch(int next_pid) {
  int old_process_pid = -1;
  if (current_process != NULL) {
    current_process->base = cpu_mmu.base;
    current_process->bound = cpu_mmu.bound;
    old_process_pid = current_process->pid;
  }

  current_process = &process_table[next_pid];

  cpu_mmu.base = current_process->base;
  cpu_mmu.bound = current_process->bound;

  printf("Switched from process %d to %d", old_process_pid,
         current_process->pid);
}

int translate(int virtual_address) {
  if (virtual_address < 0) {
    fprintf(stderr, "virtual address is negative");
    return -1;
  }

  if (virtual_address >= cpu_mmu.bound) {
    fprintf(stderr, "virtual address more than CPU bound");
    return -1;
  }
  return cpu_mmu.base + virtual_address;
}

void print_mmu_state(void) {
  printf("-- CPU MMU State --\n");
  printf("Base: %d\n", cpu_mmu.base);
  printf("Bounds: %d\n", cpu_mmu.bound);
  if (current_process != NULL) {
    printf("Current PID: %d\n", current_process->pid);
  } else {
    printf("No process running.\n");
  }
  printf("\n");
}

void print_process_table(void) {
  printf("-- Process Table --\n");
  printf("PID\tIn Use\tBase\tBounds\n");
  for (int i = 0; i < MAX_PROCESS; i++) {
    if (process_table[i].is_used) {
      printf("%d\t%s\t%d\t%d\n", process_table[i].pid, "Yes",
             process_table[i].base, process_table[i].bound);
    } else {
      printf("%d\t%s\t-\t-\n", i, "No");
    }
  }
  printf("\n");
}

void print_mem_map(void) {
  printf("-- Physical Memory Map --\n");
  if (PHYSICAL_MEMORY == 0) {
    printf("Memory is empty.\n");
    return;
  }

  char current_char = physical_memory[0];
  int block_start = 0;

  for (int i = 1; i <= PHYSICAL_MEMORY; i++) {
    if (i == PHYSICAL_MEMORY || physical_memory[i] != current_char) {
      printf("[%4d - %4d]: %c", block_start, i - 1, current_char);

      if (current_char == '.')
        printf(" (Free)");
      else if (current_char == 'A')
        printf(" (Allocated)");
      else
        printf(" (Unknown)");

      printf("\n");

      if (i < PHYSICAL_MEMORY) {
        current_char = physical_memory[i];
        block_start = i;
      }
    }
  }
  printf("\n");
}
