/* 
 *  Multi2Sim
 *  Copyright (C) 2012  Rafael Ubal (ubal@ece.neu.edu)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <assert.h>
#include <ctype.h>

#include <lib/mhandle/mhandle.h>
#include <lib/util/debug.h>
#include <lib/util/list.h>
#include <lib/util/misc.h>
#include <lib/util/string.h>

#include "asm.h"
#include "bin-file.h"
#include "inst.h"


struct si_inst_info_t si_inst_info[SI_INST_COUNT];


/* Pointers to 'si_inst_info' table indexed by instruction opcode */
struct si_inst_info_t *si_inst_info_sopp[SI_INST_INFO_SOPP_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_sopc[SI_INST_INFO_SOPC_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_sop1[SI_INST_INFO_SOP1_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_sopk[SI_INST_INFO_SOPK_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_sop2[SI_INST_INFO_SOP2_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_smrd[SI_INST_INFO_SMRD_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_vop3[SI_INST_INFO_VOP3_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_vopc[SI_INST_INFO_VOPC_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_vop1[SI_INST_INFO_VOP1_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_vop2[SI_INST_INFO_VOP2_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_vintrp[SI_INST_INFO_VINTRP_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_ds[SI_INST_INFO_DS_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_mtbuf[SI_INST_INFO_MTBUF_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_mubuf[SI_INST_INFO_MUBUF_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_mimg[SI_INST_INFO_MIMG_MAX_VALUE + 1];
struct si_inst_info_t *si_inst_info_exp[SI_INST_INFO_EXP_MAX_VALUE + 1];


struct str_map_t si_inst_fmt_map =
{
	18,
	{
		{ "<invalid>", SI_FMT_NONE },
		{ "sop2", SI_FMT_SOP2 },
		{ "sopk", SI_FMT_SOPK },
		{ "sop1", SI_FMT_SOP1 },
		{ "sopc", SI_FMT_SOPC },
		{ "sopp", SI_FMT_SOPP },
		{ "smrd", SI_FMT_SMRD },
		{ "vop2", SI_FMT_VOP2 },
		{ "vop1", SI_FMT_VOP1 },
		{ "vopc", SI_FMT_VOPC },
		{ "vop3a", SI_FMT_VOP3a },
		{ "vop3b", SI_FMT_VOP3b },
		{ "vintrp", SI_FMT_VINTRP },
		{ "ds", SI_FMT_DS },
		{ "mubuf", SI_FMT_MUBUF },
		{ "mtbuf", SI_FMT_MTBUF },
		{ "mimg", SI_FMT_MIMG },
		{ "exp", SI_FMT_EXP }
	}
};

/* String maps for assembly dump. */
struct str_map_t si_inst_sdst_map = {
	24, {
		{"reserved", 0},
		{"reserved", 1},
		{"vcc_lo", 2},
		{"vcc_hi", 3},
		{"tba_lo", 4},
		{"tba_hi", 5},
		{"tma_lo", 6},
		{"tma_hi", 7},
		{"ttmp0", 8},
		{"ttmp1", 9},
		{"ttmp2", 10},
		{"ttmp3", 11},
		{"ttmp4", 12},
		{"ttmp5", 13},
		{"ttmp6", 14},
		{"ttmp7", 15},
		{"ttmp8", 16},
		{"ttmp9", 17},
		{"ttmp10", 18},
		{"ttmp11", 19},
		{"m0", 20},
		{"reserved", 21},
		{"exec_lo", 22},
		{"exec_hi", 23}
	}
};

struct str_map_t si_inst_ssrc_map = {
	16, {
		{"0.5", 0},
		{"-0.5", 1},
		{"1.0", 2},
		{"-1.0", 3},
		{"2.0", 4},
		{"-2.0", 5},
		{"4.0", 6},
		{"-4.0", 7},
		{"reserved", 8},
		{"reserved", 9},
		{"reserved", 10},
		{"vccz", 11},
		{"execz", 12},
		{"scc", 13},
		{"reserved", 14},
		{"literal constant", 15}
	}
};

struct str_map_t si_inst_buf_data_format_map = {
	16, {
		{"invalid", si_inst_buf_data_format_invalid },
		{"BUF_DATA_FORMAT_8", si_inst_buf_data_format_8 },
		{"BUF_DATA_FORMAT_16", si_inst_buf_data_format_16 },
		{"BUF_DATA_FORMAT_8_8", si_inst_buf_data_format_8_8 },
		{"BUF_DATA_FORMAT_32", si_inst_buf_data_format_32 },
		{"BUF_DATA_FORMAT_16_16", si_inst_buf_data_format_16_16 },
		{"BUF_DATA_FORMAT_10_11_11", si_inst_buf_data_format_10_11_11 },
		{"BUF_DATA_FORMAT_11_10_10", si_inst_buf_data_format_11_10_10 },
		{"BUF_DATA_FORMAT_10_10_10_2", si_inst_buf_data_format_10_10_10_2 },
		{"BUF_DATA_FORMAT_2_10_10_10", si_inst_buf_data_format_2_10_10_10 },
		{"BUF_DATA_FORMAT_8_8_8_8", si_inst_buf_data_format_8_8_8_8 },
		{"BUF_DATA_FORMAT_32_32", si_inst_buf_data_format_32_32 },
		{"BUF_DATA_FORMAT_16_16_16_16", si_inst_buf_data_format_16_16_16_16 },
		{"BUF_DATA_FORMAT_32_32_32", si_inst_buf_data_format_32_32_32 },
		{"BUF_DATA_FORMAT_32_32_32_32", si_inst_buf_data_format_32_32_32_32 },
		{"reserved", si_inst_buf_data_format_reserved }
	}
};

struct str_map_t si_inst_buf_num_format_map = {
	14, {
		{"BUF_NUM_FORMAT_UNORM", si_inst_buf_num_format_unorm },
		{"BUF_NUM_FORMAT_SNORM", si_inst_buf_num_format_snorm },
		{"BUF_NUM_FORMAT_UNSCALED", si_inst_buf_num_format_unscaled },
		{"BUF_NUM_FORMAT_SSCALED", si_inst_buf_num_format_sscaled },
		{"BUF_NUM_FORMAT_UINT", si_inst_buf_num_format_uint },
		{"BUF_NUM_FORMAT_SINT", si_inst_buf_num_format_sint },
		{"BUF_NUM_FORMAT_SNORM_NZ", si_inst_buf_num_format_snorm_nz },
		{"BUF_NUM_FORMAT_FLOAT", si_inst_buf_num_format_float },
		{"reserved", si_inst_buf_num_format_reserved },
		{"BUF_NUM_FORMAT_SRGB", si_inst_buf_num_format_srgb },
		{"BUF_NUM_FORMAT_UBNORM", si_inst_buf_num_format_ubnorm },
		{"BUF_NUM_FORMAT_UBNORM_NZ", si_inst_buf_num_format_ubnorm_nz },
		{"BUF_NUM_FORMAT_UBINT", si_inst_buf_num_format_ubint },
		{"BUF_NUM_FORMAT_UBSCALED", si_inst_buf_num_format_ubscaled }
	}
};

struct str_map_t si_inst_OP16_map = {
	16, {
		{"f", 0},
		{"lt", 1},
		{"eq", 2},
		{"le", 3},
		{"gt", 4},
		{"lg", 5},
		{"ge", 6},
		{"o", 7},
		{"u", 8},
		{"nge", 9},
		{"nlg", 10},
		{"ngt", 11},
		{"nle", 12},
		{"neq", 13},
		{"nlt", 14},
		{"tru", 15},
	}
};

struct str_map_t si_inst_OP8_map = {
	8, {
		{"f", 0},
		{"lt", 1},
		{"eq", 2},
		{"le", 3},
		{"gt", 4},
		{"lg", 5},
		{"ge", 6},
		{"tru", 7},
	}
};

struct str_map_t si_inst_special_reg_map = {
	4, {
		{ "vcc", si_inst_special_reg_vcc },
		{ "scc", si_inst_special_reg_scc },
		{ "exec", si_inst_special_reg_exec },
		{ "tma", si_inst_special_reg_tma }
	}
};




/* 
 * Initialization/finalization of disassembler
 */

void si_disasm_init()
{
	struct si_inst_info_t *info;
	int i;

	/* Type size assertions */
	assert(sizeof(union si_reg_t) == 4);

	/* Read information about all instructions */
#define DEFINST(_name, _fmt_str, _fmt, _opcode, _size, _flags) \
	info = &si_inst_info[SI_INST_##_name]; \
	info->inst = SI_INST_##_name; \
	info->name = #_name; \
	info->fmt_str = _fmt_str; \
	info->fmt = SI_FMT_##_fmt; \
	info->opcode = _opcode; \
	info->size = _size; \
	info->flags = _flags;
#include "asm.dat"
#undef DEFINST

	/* Tables of pointers to 'si_inst_info' */
	for (i = 1; i < SI_INST_COUNT; i++)
	{
		info = &si_inst_info[i];

		if (info->fmt == SI_FMT_SOPP)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOPP_MAX_VALUE));
			si_inst_info_sopp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPC)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOPC_MAX_VALUE));
			si_inst_info_sopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP1)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOP1_MAX_VALUE));
			si_inst_info_sop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOPK)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOPK_MAX_VALUE));
			si_inst_info_sopk[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SOP2)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SOP2_MAX_VALUE));
			si_inst_info_sop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_SMRD) 
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_SMRD_MAX_VALUE));
			si_inst_info_smrd[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP3a || info->fmt == SI_FMT_VOP3b)
		{
			int i;

			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOP3_MAX_VALUE));
			si_inst_info_vop3[info->opcode] = info;
			if (info->flags & SI_INST_FLAG_OP8)
			{
				for (i = 1; i < 8; i++)
				{
					si_inst_info_vop3[info->opcode + i] = 
						info;
				}
			}
			if (info->flags & SI_INST_FLAG_OP16)
			{
				for (i = 1; i < 16; i++)
				{
					si_inst_info_vop3[info->opcode + i] = 
						info;
				}
			}
			continue;
		}
		else if (info->fmt == SI_FMT_VOPC)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOPC_MAX_VALUE));
			si_inst_info_vopc[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP1)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOP1_MAX_VALUE));
			si_inst_info_vop1[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VOP2)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VOP2_MAX_VALUE));
			si_inst_info_vop2[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_VINTRP)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_VINTRP_MAX_VALUE));
			si_inst_info_vintrp[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_DS)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_DS_MAX_VALUE));
			si_inst_info_ds[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MTBUF)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_MTBUF_MAX_VALUE));
			si_inst_info_mtbuf[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MUBUF)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_MUBUF_MAX_VALUE));
			si_inst_info_mubuf[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_MIMG)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_MIMG_MAX_VALUE));
			si_inst_info_mimg[info->opcode] = info;
			continue;
		}
		else if (info->fmt == SI_FMT_EXP)
		{
			assert(IN_RANGE(info->opcode, 0, 
				SI_INST_INFO_EXP_MAX_VALUE));
			si_inst_info_exp[info->opcode] = info;
			continue;
		}
		else 
		{
			fprintf(stderr, "warning: '%s' not indexed\n", 
				info->name);
		}
	}
}


void si_disasm_done()
{

}



/* 
 * Functions for decoding instructions
 */

void si_disasm_buffer(struct elf_buffer_t *buffer, FILE *f)
{
	void *inst_buf = buffer->ptr;
	int inst_count = 0;
	int rel_addr = 0;

	int label_addr[buffer->size / 4];	/* A list of created labels sorted by rel_addr. */

	int *next_label = &label_addr[0];	/* The next label to dump. */
	int *end_label = &label_addr[0];	/* The address after the last label. */


	/* Read through instructions to find labels. */
	while (inst_buf)
	{
		SIInst inst;
		int inst_size;

		/* Zero-out instruction structure */
		memset(&inst, 0, sizeof(SIInst));

		/* Decode instruction */
		inst_size = SIInstDecode(&inst, inst_buf, rel_addr);

		/* If ENDPGM, break. */
		if (inst.info->fmt == SI_FMT_SOPP && 
			inst.micro_inst.sopp.op == 1)
		{
			break;
		}
		/* If the instruction branches, insert the label into 
		 * the sorted list. */
		if (inst.info->fmt == SI_FMT_SOPP &&
			(inst.micro_inst.sopp.op >= 2 && 
			 inst.micro_inst.sopp.op <= 9))
		{
			short simm16 = inst.micro_inst.sopp.simm16;
			int se_simm = simm16;
			int label = rel_addr + (se_simm * 4) + 4;

			/* Find position to insert label. */
			int *t_label = &label_addr[0];

			while (t_label < end_label && *t_label < label)
				t_label++;

			if (label != *t_label || t_label == end_label)
			{

				/* Shift labels after position down. */
				int *t2_label = end_label;

				while (t2_label > t_label)
				{
					*t2_label = *(t2_label - 1);
					t2_label--;
				}
				end_label++;

				/* Insert the new label. */
				*t_label = label;
			}

		}

		inst_buf += inst_size;
		rel_addr += inst_size;
	}


	/* Reset to disassemble. */
	inst_buf = buffer->ptr;
	rel_addr = 0;

	/* Disassemble */
	while (inst_buf)
	{
		SIInst inst;
		int inst_size;

		/* Parse the instruction */
		inst_size = SIInstDecode(&inst, inst_buf, rel_addr);

		inst_count++;

		/* Dump a label if necessary. */
		if (*next_label == rel_addr && next_label != end_label)
		{
			fprintf(f, "label_%04X:\n", rel_addr / 4);
			next_label++;
		}


		/* Dump the instruction */
		int line_size = MAX_INST_STR_SIZE;
		char line[line_size];

		si_inst_dump(&inst, inst_size, rel_addr, inst_buf, line, 
			line_size);
		fprintf(f, " %s", line);


		/* Break at end of program. */
		if (inst.info->fmt == SI_FMT_SOPP && 
			inst.micro_inst.sopp.op == 1)
		{
			break;
		}

		/* Increment instruction pointer */
		inst_buf += inst_size;
		rel_addr += inst_size;
	}

}


/* GPU disassembler tool */
void si_disasm(char *path)
{
	struct elf_file_t *elf_file;
	struct elf_symbol_t *symbol;
	struct elf_section_t *section;
	struct si_bin_file_t *amd_bin;

	char kernel_name[MAX_STRING_SIZE];

	int i;

	/* Initialize disassembler */
	si_disasm_init();

	/* Decode external ELF */
	elf_file = elf_file_create_from_path(path);
	for (i = 0; i < list_count(elf_file->symbol_table); i++)
	{
		/* Get symbol and section */
		symbol = list_get(elf_file->symbol_table, i);
		section = list_get(elf_file->section_list, symbol->section);
		if (!section)
			continue;

		/* If symbol is '__OpenCL_XXX_kernel', it points 
		 * to internal ELF */
		if (str_prefix(symbol->name, "__OpenCL_") && 
			str_suffix(symbol->name, "_kernel"))
		{
			/* Decode internal ELF */
			str_substr(kernel_name, sizeof(kernel_name), 
				symbol->name, 9, strlen(symbol->name) - 16);
			amd_bin = si_bin_file_create(
				section->buffer.ptr + symbol->value, 
				symbol->size, kernel_name);

			/* Get kernel name */
			printf("**\n** Disassembly for '__kernel %s'\n**\n\n",
				kernel_name);
			si_disasm_buffer( 
				&amd_bin->enc_dict_entry_southern_islands->
				sec_text_buffer, stdout);
			printf("\n\n\n");

			/* Free internal ELF */
			si_bin_file_free(amd_bin);
		}
	}

	/* Free external ELF */
	elf_file_free(elf_file);
	si_disasm_done();

	/* End */
	mhandle_done();
	exit(0);
}
