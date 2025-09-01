#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_PROCS 2
#define PM_SIZE (64 * 1024)
#define OP_NOP 0
#define OP_INC 1
#define OP_DEC 2
#define OP_YIELD 3

typedef struct address_space {
  size_t code_base_address;
  size_t heap_base_address;
  size_t stack_base_address;
} address_space;

typedef struct PCB {
  int process_id;
  uint16_t pc;
  int registers[4];
  char state;
  address_space process_address_space;
} PCB;

char physical_memory[PM_SIZE];
int current_process = 0;
PCB processes_pcb[NUM_PROCS];

void initialize_os(void);
void initialize_process(int process_id, size_t memory_start);
size_t translate_address(int process_id, size_t v_addr);
void run_cpu(int process_id);
void scheduler(void);

void initialize_os(void) {
  for (size_t i = 0; i < PM_SIZE; ++i) {
    physical_memory[i] = 0;
  }
  initialize_process(0, 0x100);
  initialize_process(1, 0x500);

  current_process = 0;
  processes_pcb[0].state = 'R';
  processes_pcb[1].state = 'Y';
}

void initialize_process(int process_id, size_t memory_start) {
  PCB *pcb = &processes_pcb[process_id];
  pcb->process_id = process_id;
  pcb->pc = 0;
  pcb->state = 'Y';
  for (size_t i = 0; i < 4; ++i) {
    pcb->registers[i] = 0;
  }
  pcb->process_address_space.code_base_address = memory_start;
  pcb->process_address_space.heap_base_address = memory_start + 0x100;
  pcb->process_address_space.stack_base_address = memory_start + 0x200;

  if (process_id == 0) {
    physical_memory[memory_start + 0] = OP_INC;
    physical_memory[memory_start + 1] = OP_INC;
    physical_memory[memory_start + 2] = OP_YIELD;
    physical_memory[memory_start + 3] = 99;
  } else if (process_id == 1) {
    physical_memory[memory_start + 0] = OP_DEC;
    physical_memory[memory_start + 1] = OP_YIELD;
    physical_memory[memory_start + 2] = OP_DEC;
    physical_memory[memory_start + 3] = 99;
  }
}

size_t translate_address(int process_id, size_t v_addr) {
  return processes_pcb[process_id].process_address_space.code_base_address +
         v_addr;
}

void run_cpu(int process_id) {
  PCB *process_pcb = &processes_pcb[process_id];
  printf("Running Process ID : %d, with program counter %d\n", process_id,
         process_pcb->pc);

  for (size_t i = 0; i < 3; i++) {
    size_t physical_addr = translate_address(process_id, process_pcb->pc);
    char opcode = physical_memory[physical_addr];

    printf(
        "P%d: Executing instruction %d at virtual address %d (physical %zu)\n",
        process_id, opcode, process_pcb->pc, physical_addr);

    switch (opcode) {
    case OP_NOP:
      process_pcb->pc++;
      break;
    case OP_INC:
      process_pcb->registers[0]++;
      printf("  R0 now = %d\n", process_pcb->registers[0]);
      process_pcb->pc++;
      break;
    case OP_DEC:
      process_pcb->registers[0]--;
      printf("  R0 now = %d\n", process_pcb->registers[0]);
      process_pcb->pc++;
      break;
    case OP_YIELD:
      process_pcb->pc++;
      printf("  Yielding CPU\n");
      return;
    default:
      printf("  Unknown opcode %d! Terminating process\n", opcode);
      process_pcb->state = 'T';
      return;
    }

    if (process_pcb->pc > 10) {
      printf("Process %d completed execution\n", process_id);
      process_pcb->state = 'T';
      return;
    }
  }
  printf("CPU: Time slice expired for P%d.\n", process_id);
}

void scheduler(void) {
  int next_process = current_process;
  int found = 0;

  for (int i = 1; i <= NUM_PROCS; i++) {
    int candidate = (current_process + i) % NUM_PROCS;
    if (processes_pcb[candidate].state == 'Y' ||
        processes_pcb[candidate].state == 'R') {
      next_process = candidate;
      found = 1;
      break;
    }
  }

  if (!found) {
    return;
  }

  if (next_process != current_process) {
    printf("Scheduler: Switching from P%d to P%d\n", current_process,
           next_process);
    if (processes_pcb[current_process].state != 'T') {
      processes_pcb[current_process].state = 'Y';
    }
    current_process = next_process;
    processes_pcb[current_process].state = 'R';
  } else {
    printf("Scheduler: Continuing with P%d\n", current_process);
  }
}

int main(void) {
  printf("==== OS Initialization ====\n");
  initialize_os();

  printf("\n==== Starting Execution ====\n");
  while (1) {
    if (processes_pcb[current_process].state == 'T') {
      printf("Process %d terminated, calling scheduler\n", current_process);
      scheduler();
      if (processes_pcb[current_process].state == 'T') {
        printf("All processes terminated. Exiting.\n");
        break;
      }
    }

    printf("\nKernel: Executing process %d\n", current_process);
    run_cpu(current_process);
    scheduler();
  }

  return 0;
}
