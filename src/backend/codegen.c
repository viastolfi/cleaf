#include "codegen.h"

static int CODEGEN_get_var_pos(var_array* vars, const char* name) 
{
  da_foreach(var_pair_t, it, vars) {
    if (strcmp(it->name, name) == 0)
      return it->place;
  }

  return -1;
}

static const char* CODEGEN_get_reg(const target_t* target, int id)
{
  if (id < 0)
    return target->reserved_regs[-1 - id];
  return target->regs_8[id % target->reg_8_count];
}

int CODEGEN_write_function(
    string_builder_t* sb,
    IR_function_t* func,
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
  target->emit_stack_setup(sb, func->stack_reserve_size);

  da_foreach(IR_instruction_t*, it, func->code) {
    switch ((*it)->kind) {
    case IR_LOAD_VAR:
      {
        // since this append after semantic analyze this can't fail
        int place = CODEGEN_get_var_pos(&vars, (*it)->var.name);
        target->emit_mov_from_stack(sb, 
            CODEGEN_get_reg(target, (*it)->dest.id), place);
      }
      break;
    case IR_STORE_VAR:
      int place = CODEGEN_get_var_pos(&vars, (*it)->var.name);
      if (place == -1) {
        char* name = strdup((*it)->var.name);  
        place = actual_place++ * 8;
        var_pair_t pair = {name, place};
        da_append(&vars, pair); 
      }
      // TODO: handle uninitialized var
      if ((*it)->var.is_init) {
        target->emit_mov_at_stack(sb, place, 
            CODEGEN_get_reg(target, (*it)->src.id));
      }
      break;
    case IR_INC:
      target->emit_inc(sb, CODEGEN_get_reg(target, (*it)->dest.id));
      break;
    case IR_DEC:
      target->emit_dec(sb, CODEGEN_get_reg(target, (*it)->dest.id));
      break;
    case IR_CHUNK:
      target->chunk_write(sb, (*it)->chunk_name);
      break;
    case IR_JMP:
      target->emit_jmp(sb, (*it)->chunk_name);
      break;
    case IR_JMP_EQUAL:
      target->emit_jmp_equal(sb, (*it)->chunk_name);
      break;
    case IR_JMP_NOT_EQUAL:
      target->emit_jmp_not_equal(sb, (*it)->chunk_name);
      break;
    case IR_JMP_GREATER_THAN:
      target->emit_jmp_greater_than(sb, (*it)->chunk_name);
      break;
    case IR_JMP_GREATER_THAN_EQUAL:
      target->emit_jmp_greater_than_equal(sb, (*it)->chunk_name);
      break;
    case IR_JMP_LOWER_THAN:
      target->emit_jmp_lower_than(sb, (*it)->chunk_name);
      break;
    case IR_JMP_LOWER_THAN_EQUAL:
      target->emit_jmp_lower_than_equal(sb, (*it)->chunk_name);
      break;
    case IR_BINARY:
      if ((*it)->binary_op == IR_BINARY_CMP) {
        target->emit_cmp(sb, 
            CODEGEN_get_reg(target, (*it)->src.id), 
            CODEGEN_get_reg(target, (*it)->dest.id));
      } else if ((*it)->binary_op == IR_BINARY_ADD) {
        target->emit_add(sb, 
            CODEGEN_get_reg(target, (*it)->dest.id),
            CODEGEN_get_reg(target, (*it)->src.id));
      } else if ((*it)->binary_op == IR_BINARY_SUB) {
        target->emit_sub(sb,
            CODEGEN_get_reg(target, (*it)->dest.id),
            CODEGEN_get_reg(target, (*it)->src.id));
      } else if ((*it)->binary_op == IR_BINARY_MUL) {
        target->emit_mul(sb,
            CODEGEN_get_reg(target, (*it)->dest.id),
            CODEGEN_get_reg(target, (*it)->src.id));
      } else {
        error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED,
            "binary op not yet implemented in codegen");
        return 1;
      }
      break;
    case IR_INT_CONST:
      target->emit_mov_direct(sb, 
          CODEGEN_get_reg(target, (*it)->dest.id),
          (*it)->int_value);
      break;
    case IR_MOV: {
      const char* dst = CODEGEN_get_reg(target, (*it)->dest.id);
      const char* src = CODEGEN_get_reg(target, (*it)->src.id);
      target->emit_mov(sb, dst, src);
    }
      break;
    case IR_CALL:
      target->emit_call(sb, (*it)->func_name);
      break;
    case IR_RETURN:
      target->emit_stack_restore(sb, func->stack_reserve_size);
      target->emit_ret(sb);
      break;
    case IR_EXIT:
      target->emit_stack_restore(sb, func->stack_reserve_size);
      target->emit_process_exit(sb, CODEGEN_get_reg(target, (*it)->dest.id));
      break;
    case IR_ALLOC:
      target->alloc_memory(sb, (*it)->alloc_size);
      break;
    case IR_MOV_OFFSET:
      if ((*it)->offset.timing == IR_PRE_OFFSET) {
      const char* dst = CODEGEN_get_reg(target, (*it)->dest.id);
      const char* src = CODEGEN_get_reg(target, (*it)->src.id);
        target->emit_mov_offset_pre(
            sb, dst, (*it)->offset.size, src);
        break;
      } else {
        const char* dst = CODEGEN_get_reg(target, (*it)->dest.id);
        const char* src = CODEGEN_get_reg(target, (*it)->src.id);
        target->emit_mov_offset_post(
            sb, dst, (*it)->offset.size, src);
        break;
      }
    default:
      error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED, 
          "unknown IR instruction");
      return 1;
    } 
  }

  da_foreach(var_pair_t, it, &vars)
    free(it->name); 

  da_free(&vars);

  return 0;
}
