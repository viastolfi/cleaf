#include "codegen.h"

static int CODEGEN_get_var_pos(var_array* vars, const char* name) 
{
  da_foreach(var_pair_t, it, vars) {
    if (strcmp(it->name, name) == 0)
      return it->place;
  }

  return -1;
}

static const char* CODEGEN_get_reg(
    const target_t* target, 
    IR_temp_id ir_temp_id,
    bool force_8_register) // we use this to ensure we use 8 bytes registers when used on known workflow like exit syscall
                           // this is pretty ugly
                           // TODO: find a better way of doing this
{
  if (ir_temp_id.id < 0)
    return target->reserved_regs[-1 - ir_temp_id.id];
  if (ir_temp_id.size == 8 || force_8_register) 
    return target->regs_8[ir_temp_id.id % target->reg_8_count];
  else if (ir_temp_id.size == 4) 
    return target->regs_4[ir_temp_id.id % target->reg_4_count];

  // some kind of fallback
  return target->regs_8[ir_temp_id.id % target->reg_8_count];
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
            CODEGEN_get_reg(target, (*it)->dest, false), place);
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
            CODEGEN_get_reg(target, (*it)->src, false));
      }
      break;
    case IR_INC:
      target->emit_inc(
          sb, CODEGEN_get_reg(target, (*it)->dest, false));
      break;
    case IR_DEC:
      target->emit_dec(
          sb, CODEGEN_get_reg(target, (*it)->dest, false));
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
            CODEGEN_get_reg(target, (*it)->src, false), 
            CODEGEN_get_reg(target, (*it)->dest, false));
      } else if ((*it)->binary_op == IR_BINARY_ADD) {
        target->emit_add(sb, 
            CODEGEN_get_reg(target, (*it)->dest, false),
            CODEGEN_get_reg(target, (*it)->src, false));
      } else if ((*it)->binary_op == IR_BINARY_SUB) {
        target->emit_sub(sb,
            CODEGEN_get_reg(target, (*it)->dest, false),
            CODEGEN_get_reg(target, (*it)->src, false));
      } else if ((*it)->binary_op == IR_BINARY_MUL) {
        target->emit_mul(sb,
            CODEGEN_get_reg(target, (*it)->dest, false),
            CODEGEN_get_reg(target, (*it)->src, false));
      } else {
        error_report_general(ERROR_SEVERITY_NOT_IMPLEMENTED,
            "binary op not yet implemented in codegen");
        return 1;
      }
      break;
    case IR_INT_CONST:
      target->emit_mov_direct(sb, 
          CODEGEN_get_reg(target, (*it)->dest, false),
          (*it)->int_value);
      break;
    case IR_MOV: {
      const char* dst = CODEGEN_get_reg(target, (*it)->dest, false);
      const char* src = CODEGEN_get_reg(target, (*it)->src, false);
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
      target->emit_process_exit(
          sb, CODEGEN_get_reg(target, (*it)->dest, true));
      break;
    case IR_ALLOC:
      target->alloc_memory(sb, (*it)->alloc_size);
      break;
    case IR_DEALLOC:
      const char * src = CODEGEN_get_reg(target, (*it)->src, false);
      target->dealloc_memory(sb, src, (*it)->src.size);
      break;
    case IR_DIRECT_MUL:
      target->emit_mul_direct(sb,
          CODEGEN_get_reg(target, (*it)->dest, false),
          (*it)->int_value);
      break;
    case IR_LOAD_ELEM:
      {
        const char* dst   = CODEGEN_get_reg(target, (*it)->dest,  false);
        const char* base  = CODEGEN_get_reg(target, (*it)->src,   false);
        const char* index = CODEGEN_get_reg(target, (*it)->index, false);
        target->emit_load_elem(sb, dst, base, index);
      }
      break;
    case IR_MOV_OFFSET:
      if ((*it)->offset.timing == IR_PRE_OFFSET) {
      const char* dst = CODEGEN_get_reg(target, (*it)->dest, false);
      const char* src = CODEGEN_get_reg(target, (*it)->src, false);
        target->emit_mov_offset_pre(
            sb, dst, (*it)->offset.size, src);
        break;
      } else {
        const char* dst = 
          CODEGEN_get_reg(target, (*it)->dest, false);
        const char* src = 
          CODEGEN_get_reg(target, (*it)->src, false);

        target->emit_mov_offset_post(
            sb, dst, (*it)->offset.size, src);
        break;
      }
    case IR_ASM: {
      size_t arg_idx = 0;
      for (size_t i = 0; i < (*it)->asm_data.string_count; i++) {
        const char* s = (*it)->asm_data.strings[i];
        const char* pct = strchr(s, '%');
        if (pct && arg_idx < (*it)->asm_data.arg_count) {
          const char* reg = 
            CODEGEN_get_reg(target, (*it)->asm_data.args[arg_idx++], true);
          sb_append_fmt(sb, "    %.*s%s\n", (int)(pct - s), s, reg);
        } else {
          sb_append_fmt(sb, "    %s\n", s);
        }
      }
    }
      break;
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
