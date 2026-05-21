#include "codegen.h"

static int CODEGEN_get_var_pos(var_array* vars, const char* name) 
{
  da_foreach(var_pair_t, it, vars) {
    if (strcmp(it->name, name) == 0)
      return it->place;
  }

  return -1;
}

static void CODEGEN_restore_stack(
    string_builder_t* sb,
    const target_t* target,
    int size)
{
  target->emit_add(sb, "rsp", size);
  target->emit_pop(sb, "rbp");
}

int CODEGEN_write_function(
    string_builder_t* sb,
    HIR_function_t* func,
    const target_t* target)
{
  var_array vars = {0};
  int actual_place = 1;

  if (strcmp(func->name, "main") == 0) {
    target->setup(sb);
    target->func_write(sb, "start"); 
  }
  else {
    target->func_write(sb, func->name);
  }

  // stack prep
  target->emit_push(sb, "rbp");
  target->emit_mov(sb, "rbp", "rsp");
  target->emit_sub_direct(sb, "rsp", func->stack_reserve_size);

  da_foreach(HIR_instruction_t*, it, func->code) {
    switch ((*it)->kind) {
    case HIR_STORE_VAR:
      int place = CODEGEN_get_var_pos(&vars, (*it)->var.name);
      if (place == -1) {
        char* name = strdup((*it)->var.name);  
        place = actual_place++ * 8;
        var_pair_t pair = {name, place};
        da_append(&vars, pair); 
      }
      // TODO: handle uninitialized var
      if ((*it)->var.is_init) {
        target->emit_mov_at_stack(sb, place, target->regs[(*it)->a]);
      }
      break;
    case HIR_INT_CONST:
      target->emit_mov_direct(sb, 
          target->regs[(*it)->dest], (*it)->int_value);
      break;
    case HIR_EXIT:
      CODEGEN_restore_stack(sb, target, func->stack_reserve_size);
      target->emit_mov_direct(sb, "rax", 60);
      target->emit_mov(sb, "rdi", target->regs[(*it)->dest]);
      target->emit_syscall(sb);
      break;
    default:
      error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED, 
          "unknown HIR instruction");
      return 1;
    } 
  }

  da_foreach(var_pair_t, it, &vars)
    free(it->name); 

  return 0;
}
