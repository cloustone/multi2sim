/*
 *  Multi2Sim Tools
 *  Copyright (C) 2011  Rafael Ubal (ubal@ece.neu.edu)
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

#include <visual-evergreen.h>


/*
 * Trace Line Processing Functions
 */


static void vi_evg_gpu_new_ndrange(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
}


static void vi_evg_gpu_new_work_group(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
}


static void vi_evg_gpu_new_wavefront(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
}


/* Command 'evg.map_work_group'
 * 	wg=<id>		(e.g., 2)
 * 	wi_first=<id>
 * 	wi_count=<num>
 * 	wf_first=<id>
 * 	wf_count=<num>
 */
static void vi_evg_gpu_map_work_group(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_evg_work_group_t *work_group;
	struct vi_evg_compute_unit_t *compute_unit;

	char work_group_name[MAX_STRING_SIZE];

	int work_group_id;
	int compute_unit_id;

	int work_item_id_first;
	int work_item_count;

	int wavefront_id_first;
	int wavefront_count;

	/* Fields */
	work_group_id = vi_trace_line_get_symbol_int(trace_line, "wg");
	compute_unit_id = vi_trace_line_get_symbol_int(trace_line, "cu");
	work_item_id_first = vi_trace_line_get_symbol_int(trace_line, "wi_first");
	work_item_count = vi_trace_line_get_symbol_int(trace_line, "wi_count");
	wavefront_id_first = vi_trace_line_get_symbol_int(trace_line, "wf_first");
	wavefront_count = vi_trace_line_get_symbol_int(trace_line, "wf_count");

	/* Create work-group */
	snprintf(work_group_name, sizeof work_group_name, "wg-%d", work_group_id);
	work_group = vi_evg_work_group_create(work_group_name, work_group_id,
		work_item_id_first, work_item_count, wavefront_id_first, wavefront_count);

	/* Compute unit */
	compute_unit = list_get(vi_evg_gpu->compute_unit_list, compute_unit_id);
	if (!compute_unit)
		panic("%s: invalid compute unit id", __FUNCTION__);

	/* Insert work-group */
	if (!hash_table_insert(compute_unit->work_group_table, work_group_name, work_group))
		panic("%s: invalid work-group", __FUNCTION__);
}


static void vi_evg_gpu_unmap_work_group(struct vi_evg_gpu_t *gpu,
	struct vi_trace_line_t *trace_line)
{
	struct vi_evg_compute_unit_t *compute_unit;
	struct vi_evg_work_group_t *work_group;

	int work_group_id;
	int compute_unit_id;

	char work_group_name[MAX_STRING_SIZE];

	/* Fields */
	compute_unit_id = vi_trace_line_get_symbol_int(trace_line, "cu");
	work_group_id = vi_trace_line_get_symbol_int(trace_line, "wg");

	/* Compute unit */
	compute_unit = list_get(vi_evg_gpu->compute_unit_list, compute_unit_id);
	if (!compute_unit)
		panic("%s: invalid compute unit id", __FUNCTION__);

	/* Unmap work-group */
	snprintf(work_group_name, sizeof work_group_name, "wg-%d", work_group_id);
	work_group = hash_table_remove(compute_unit->work_group_table, work_group_name);
	if (!work_group)
		panic("%s: invalid work-group", __FUNCTION__);
	vi_evg_work_group_free(work_group);
}


/* Command 'evg.new_inst'.
 * 	id=<inst-id>	(e.g., 23)
 * 	cu=<cu-id>	(e.g., 4)
 * 	wg=<wg-id>	(e.g., 1)
 * 	wf=<wf-id>	(e.g., 2)
 * 	cat=<category>	(e.g., "cf")
 * 	asm=<code>	(e.g., "ALU_BREAK")
 * 	x=<code>
 * 	y=<code>
 * 	z=<code>
 * 	w=<code>
 * 	t=<code>
 */
static void vi_evg_gpu_new_inst(struct vi_evg_gpu_t *gpu, struct vi_trace_line_t *trace_line)
{
	struct vi_evg_inst_t *inst;
	struct vi_evg_compute_unit_t *compute_unit;

	char inst_name[MAX_STRING_SIZE];

	char *asm_code;
	char *asm_code_x;
	char *asm_code_y;
	char *asm_code_z;
	char *asm_code_w;
	char *asm_code_t;

	long long inst_id;
	int compute_unit_id;
	int wavefront_id;
	int work_group_id;
	enum vi_evg_inst_cat_t cat;

	/* Fields */
	inst_id = vi_trace_line_get_symbol_long_long(trace_line, "id");
	compute_unit_id = vi_trace_line_get_symbol_int(trace_line, "cu");
	work_group_id = vi_trace_line_get_symbol_int(trace_line, "wg");
	wavefront_id = vi_trace_line_get_symbol_int(trace_line, "wf");
	cat = map_string(&vi_evg_inst_cat_map, vi_trace_line_get_symbol(trace_line, "cat"));
	asm_code = vi_trace_line_get_symbol(trace_line, "asm");
	asm_code_x = vi_trace_line_get_symbol(trace_line, "x");
	asm_code_y = vi_trace_line_get_symbol(trace_line, "y");
	asm_code_z = vi_trace_line_get_symbol(trace_line, "z");
	asm_code_w = vi_trace_line_get_symbol(trace_line, "w");
	asm_code_t = vi_trace_line_get_symbol(trace_line, "t");

	/* Create */
	snprintf(inst_name, sizeof inst_name, "i-%lld", inst_id);
	inst = vi_evg_inst_create(inst_name, inst_id, compute_unit_id, work_group_id,
		wavefront_id, cat, asm_code, asm_code_x, asm_code_y, asm_code_z,
		asm_code_w, asm_code_t);

	/* Get compute unit */
	compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);
	if (!compute_unit)
		panic("%s: invalid compute unit", __FUNCTION__);

	/* Insert in compute unit */
	compute_unit->num_insts = MAX(compute_unit->num_insts, inst_id + 1);
	if (!hash_table_insert(compute_unit->inst_table, inst_name, inst))
		panic("%s: invalid instruction", __FUNCTION__);
}


/* Command 'evg.end_inst'
 * 	id=<inst-id>	(e.g., 23)
 * 	cu=<cu-id>	(e.g., 4)
 */
static void vi_evg_gpu_end_inst(struct vi_evg_gpu_t *gpu, struct vi_trace_line_t *trace_line)
{
	char inst_name[MAX_STRING_SIZE];

	long long inst_id;

	struct vi_evg_inst_t *inst;
	struct vi_evg_compute_unit_t *compute_unit;

	int compute_unit_id;

	/* Fields */
	inst_id = vi_trace_line_get_symbol_long_long(trace_line, "id");
	compute_unit_id = vi_trace_line_get_symbol_int(trace_line, "cu");

	/* Get compute unit */
	compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);
	if (!compute_unit)
		panic("%s: invalid compute unit", __FUNCTION__);

	/* Remove */
	snprintf(inst_name, sizeof inst_name, "i-%lld", inst_id);
	inst = hash_table_remove(compute_unit->inst_table, inst_name);
	if (!inst)
		panic("%s: invalid instruction", __FUNCTION__);
	vi_evg_inst_free(inst);
}


static void vi_evg_gpu_read_checkpoint(struct vi_evg_gpu_t *gpu, FILE *f)
{
	int compute_unit_id;

	struct vi_evg_compute_unit_t *compute_unit;

	/* Compute units */
	LIST_FOR_EACH(gpu->compute_unit_list, compute_unit_id)
	{
		compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);
		vi_evg_compute_unit_read_checkpoint(compute_unit, f);
	}
}


static void vi_evg_gpu_write_checkpoint(struct vi_evg_gpu_t *gpu, FILE *f)
{
	int compute_unit_id;

	struct vi_evg_compute_unit_t *compute_unit;

	/* Compute units */
	LIST_FOR_EACH(gpu->compute_unit_list, compute_unit_id)
	{
		compute_unit = list_get(gpu->compute_unit_list, compute_unit_id);
		vi_evg_compute_unit_write_checkpoint(compute_unit, f);
	}
}




/*
 * Public Functions
 */


struct vi_evg_gpu_t *vi_evg_gpu;


void vi_evg_gpu_init(void)
{
	struct vi_trace_line_t *trace_line;

	/* Allocate */
	vi_evg_gpu = calloc(1, sizeof(struct vi_evg_gpu_t));
	if (!vi_evg_gpu)
		fatal("%s: out of memory", __FUNCTION__);

	/* Initialize */
	vi_evg_gpu->work_group_table = hash_table_create(0, FALSE);
	vi_evg_gpu->compute_unit_list = list_create();

	/* State file */
	vi_state_new_category("Evergreen GPU",
		(vi_state_read_checkpoint_func_t) vi_evg_gpu_read_checkpoint,
		(vi_state_write_checkpoint_func_t) vi_evg_gpu_write_checkpoint,
		vi_evg_gpu);

	/* Commands */
	vi_state_new_command("evg.new_ndrange",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_ndrange,
		vi_evg_gpu);
	vi_state_new_command("evg.new_wg",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_work_group,
		vi_evg_gpu);
	vi_state_new_command("evg.new_wf",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_wavefront,
		vi_evg_gpu);
	vi_state_new_command("evg.map_wg",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_map_work_group,
		vi_evg_gpu);
	vi_state_new_command("evg.unmap_wg",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_unmap_work_group,
		vi_evg_gpu);
	vi_state_new_command("evg.new_inst",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_new_inst,
		vi_evg_gpu);
	vi_state_new_command("evg.end_inst",
		(vi_state_process_trace_line_func_t) vi_evg_gpu_end_inst,
		vi_evg_gpu);

	/* Parse header in state file */
	VI_STATE_FOR_EACH_HEADER(trace_line)
	{
		char *command;

		/* Get command */
		command = vi_trace_line_get_command(trace_line);
		assert(strcmp(command, "c"));

		if (!strcmp(command, "evg.init"))
		{
			struct vi_evg_compute_unit_t *compute_unit;

			int num_compute_units;
			int i;

			char name[MAX_STRING_SIZE];

			/* Create module */
			num_compute_units = vi_trace_line_get_symbol_int(trace_line, "num_compute_units");
			for (i = 0; i < num_compute_units; i++)
			{
				snprintf(name, sizeof name, "CU-%d", i);
				compute_unit = vi_evg_compute_unit_create(name);
				list_add(vi_evg_gpu->compute_unit_list, compute_unit);
			}
		}
	}
}


void vi_evg_gpu_done(void)
{
	struct vi_evg_work_group_t *work_group;

	char *work_group_name;

	int i;

	/* Free work-groups */
	HASH_TABLE_FOR_EACH(vi_evg_gpu->work_group_table, work_group_name, work_group)
		vi_evg_work_group_free(work_group);
	hash_table_free(vi_evg_gpu->work_group_table);

	/* Free compute units */
	LIST_FOR_EACH(vi_evg_gpu->compute_unit_list, i)
		vi_evg_compute_unit_free(list_get(vi_evg_gpu->compute_unit_list, i));
	list_free(vi_evg_gpu->compute_unit_list);

	/* Free GPU */
	free(vi_evg_gpu);
}
