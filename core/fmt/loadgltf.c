
#include "brender.h"
#include "fmt.h"

#include "cgltf.h"

static void *cgltf_alloc_br(void *user, cgltf_size size)
{
	return BrResAllocate(user, size, BR_MEMORY_SCRATCH);
}

static void cgltf_free_br(void *user, void *ptr)
{
	(void)user;
	BrResFree(ptr);
}

static cgltf_result cgltf_file_read_br(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *file_options, const char *path, cgltf_size *size, void **data)
{
	br_size_t file_size;
	void *file_data;

	(void)memory_options;
	(void)file_options;

	file_data = BrFileLoad(NULL, path, &file_size);
	if (!file_data) {
		return cgltf_result_io_error;
	}

	*data = file_data;
	*size = (cgltf_size)file_size;

	return cgltf_result_success;
}

static void cgltf_file_release_br(const struct cgltf_memory_options *memory_options, const struct cgltf_file_options *file_options, void *data)
{
	(void)memory_options;
	(void)file_options;
	BrResFree(data);
}

br_fmt_results *BR_PUBLIC_ENTRY BrFmtGLTFLoad(const char *name, br_uint_32 flags)
{
	br_fmt_results *results;
	cgltf_result r;
	cgltf_data *data;

	/* create cgltf options */
	// clang-format off
	cgltf_options options = {
		.type = cgltf_file_type_invalid,
		.json_token_count = 0,
		.memory = (cgltf_memory_options){
			.alloc_func = cgltf_alloc_br,
			.free_func  = cgltf_free_br,
			.user_data = NULL
		},
		.file = (cgltf_file_options){
			.read = cgltf_file_read_br,
			.release = cgltf_file_release_br,
			.user_data = NULL
		}
	};
	// clang-format on

	/* parse gltf or glb */
	r = cgltf_parse_file(&options, name, &data);
	if (r != cgltf_result_success) {
		BrLogError("FMT", "Failed to read \"%s\"", name);
		return NULL;
	}

	/* allocate results */
	results = BrResAllocate(NULL, sizeof(br_fmt_results), BR_MEMORY_FMT_RESULTS);

	/* clean up */
	cgltf_free(data);

	return results;
}
