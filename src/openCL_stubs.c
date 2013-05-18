#include <caml/alloc.h>
#include <caml/bigarray.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/misc.h>
#include <caml/mlvalues.h>
#include <caml/signals.h>
#include <caml/custom.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include <assert.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

static inline void check_err(cl_int err)
{
  if (err != CL_SUCCESS)
    caml_raise_with_arg(*caml_named_value("opencl_exn_error"), Val_int(-err));
}

static inline void check_err_free(cl_int err, void *p)
{
  if (err != CL_SUCCESS)
    {
      if (p) free(p);
      check_err(err);
    }
}

#define Val_platform_id(p) (value)p
#define Platform_id_val(v) (cl_platform_id)v
#define Device_id_val(v) (cl_device_id)v

CAMLprim value caml_opencl_platform_ids(value unit)
{
  CAMLparam0();
  CAMLlocal1(ans);
  cl_uint n;
  int i;
  cl_platform_id *ids;

  check_err(clGetPlatformIDs(0, NULL, &n));
  ids = malloc(n * sizeof(cl_platform_id));
  check_err_free(clGetPlatformIDs(n, ids, NULL), ids);
  ans = caml_alloc_tuple(n);
  for (i = 0; i < n; i++)
    Store_field(ans, i, Val_platform_id(ids[i]));
  free(ids);

  CAMLreturn(ans);
}

static cl_platform_info Platform_info_val(value var)
{
  if (var == hash_variant("Profile"))
    return CL_PLATFORM_PROFILE;
  else if (var == hash_variant("Version"))
    return CL_PLATFORM_VERSION;
  else if (var == hash_variant("Name"))
    return CL_PLATFORM_NAME;
  else if (var == hash_variant("Vendor"))
    return CL_PLATFORM_VENDOR;
  else if (var == hash_variant("Extensions"))
    return CL_PLATFORM_EXTENSIONS;
  else
    assert(0);
}

CAMLprim value caml_opencl_platform_info(value _id, value var)
{
  CAMLparam2(_id, var);
  CAMLlocal1(ans);

  cl_platform_id id = Platform_id_val(_id);
  int len = 1024;
  char s[len];

  check_err(clGetPlatformInfo(id, Platform_info_val(var), len, s, NULL));
  ans = caml_copy_string(s);
  CAMLreturn(ans);
}

CAMLprim static value Val_di_cl_uint(void* var, size_t size)
{
  return Val_int(*(uint*) var);
}

CAMLprim static value Val_di_cl_ulong(void* var, size_t size)
{
  return caml_copy_int64(*(cl_ulong*) var);
}

CAMLprim static value Val_di_size_t(void* var, size_t size)
{
  return Val_int(*(uint*) var);
}

CAMLprim static value Val_di_cl_bool(void* var, size_t size)
{
  return Val_bool(*(cl_bool*) var);
}

CAMLprim static value Val_di_cl_string(void* var, size_t size)
{
  return caml_copy_string((char*) var);
}

CAMLprim static value Val_di_cl_platform_id(void* var, size_t size)
{
  return Val_platform_id((cl_platform_id) var);
}

typedef value (*postprocess_fn)(void* var, size_t size);

// Given a label CL_DEVICE_ADDRESS_BITS discards the CL_DEVICE prefix,
// lowercases the ADDRESS_BITS except for its first letter, resulting
// in Address_bits. The resulting value is then made into a hash
// variant.
static value device_info_variant(const char* key)
{
  const char* prefix = "CL_DEVICE_";
  const int prefix_len = strlen(prefix);
  if (strlen(key) > prefix_len &&
      strncmp(key, prefix, prefix_len) == 0) {
    char buffer[1024];
    int buf_idx;
    buffer[0] = key[prefix_len];
    for (buf_idx = 1; key[buf_idx + prefix_len]; ++buf_idx) {
      buffer[buf_idx] = tolower(key[buf_idx + prefix_len]);
    }
    buffer[buf_idx] = 0;
    return hash_variant(buffer);
  } else {
    // the key wasn't of form CL_DEVICE_X
    assert(0);
  }
}

static cl_platform_info Device_info(value var, postprocess_fn* postprocess)
{
#define KEY(key, type)                          \
  if (var == device_info_variant(#key)) {       \
    *postprocess = Val_di_##type;               \
    return key;                                 \
  } else
  KEY(CL_DEVICE_ADDRESS_BITS                  , cl_uint)
  KEY(CL_DEVICE_AVAILABLE                     , cl_bool)
  KEY(CL_DEVICE_COMPILER_AVAILABLE            , cl_bool)
    //KEY(CL_DEVICE_DOUBLE_FP_CONFIG              , cl_device_fp_config)
  KEY(CL_DEVICE_ENDIAN_LITTLE                 , cl_bool)
  KEY(CL_DEVICE_ERROR_CORRECTION_SUPPORT      , cl_bool)
    //KEY(CL_DEVICE_EXECUTION_CAPABILITIES        , cl_device_exec_capabilities)
    /* CL_EXEC_KERNEL - The OpenCL device can execute OpenCL kernels.  */
    /* CL_EXEC_NATIVE_KERNEL - The OpenCL device can execute native kernels.  The mandated minimum capability is CL_EXEC_KERNEL. */
  KEY(CL_DEVICE_EXTENSIONS                    , cl_string)
  KEY(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE         , cl_ulong)
    //KEY(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE         , cl_device_mem_cache_type)
  KEY(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE     , cl_uint)
  KEY(CL_DEVICE_GLOBAL_MEM_SIZE               , cl_ulong)
    //KEY(CL_DEVICE_HALF_FP_CONFIG                , cl_device_fp_config)
  KEY(CL_DEVICE_IMAGE_SUPPORT                 , cl_bool)
  KEY(CL_DEVICE_IMAGE2D_MAX_HEIGHT            , size_t)
  KEY(CL_DEVICE_IMAGE2D_MAX_WIDTH             , size_t)
  KEY(CL_DEVICE_IMAGE3D_MAX_DEPTH             , size_t)
  KEY(CL_DEVICE_IMAGE3D_MAX_HEIGHT            , size_t)
  KEY(CL_DEVICE_IMAGE3D_MAX_WIDTH             , size_t)
  KEY(CL_DEVICE_LOCAL_MEM_SIZE                , cl_ulong)
    //KEY(CL_DEVICE_LOCAL_MEM_TYPE                , cl_device_local_mem_type)
  KEY(CL_DEVICE_MAX_CLOCK_FREQUENCY           , cl_uint)
  KEY(CL_DEVICE_MAX_COMPUTE_UNITS             , cl_uint)
  KEY(CL_DEVICE_MAX_CONSTANT_ARGS             , cl_uint)
  KEY(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE      , cl_ulong)
  KEY(CL_DEVICE_MAX_MEM_ALLOC_SIZE            , cl_ulong)
  KEY(CL_DEVICE_MAX_PARAMETER_SIZE            , size_t)
  KEY(CL_DEVICE_MAX_READ_IMAGE_ARGS           , cl_uint)
  KEY(CL_DEVICE_MAX_SAMPLERS                  , cl_uint)
  KEY(CL_DEVICE_MAX_WORK_GROUP_SIZE           , size_t)
  KEY(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS      , cl_uint)
  KEY(CL_DEVICE_MAX_WORK_ITEM_SIZES           , size_t)
  KEY(CL_DEVICE_MAX_WRITE_IMAGE_ARGS          , cl_uint)
  KEY(CL_DEVICE_MEM_BASE_ADDR_ALIGN           , cl_uint)
  KEY(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE      , cl_uint)
  KEY(CL_DEVICE_NAME                          , cl_string)
  KEY(CL_DEVICE_PLATFORM                      , cl_platform_id)
  KEY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR   , cl_uint)
  KEY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT  , cl_uint)
  KEY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT    , cl_uint)
  KEY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG   , cl_uint)
  KEY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT  , cl_uint)
  KEY(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE , cl_uint)
  KEY(CL_DEVICE_PROFILE                       , cl_string)
  KEY(CL_DEVICE_PROFILING_TIMER_RESOLUTION    , size_t)
    //KEY(CL_DEVICE_QUEUE_PROPERTIES              , cl_command_queue_properties)
    /* CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE  */
    /* CL_QUEUE_PROFILING_ENABLE  These properties are described in the table for clCreateCommandQueue. The mandated minimum capability is CL_QUEUE_PROFILING_ENABLE. */
    //KEY(CL_DEVICE_SINGLE_FP_CONFIG              , cl_device_fp_config)
    /* CL_FP_DENORM - denorms are supported  */
    /* CL_FP_INF_NAN - INF and quiet NaNs are supported  */
    /* CL_FP_ROUND_TO_NEAREST - round to nearest even rounding mode supported  */
    /* CL_FP_ROUND_TO_ZERO - round to zero rounding mode supported  */
    /* CL_FP_ROUND_TO_INF - round to +ve and -ve infinity rounding modes supported  */
    /* CL_FP_FMA - IEEE754-2008 fused multiply-add is supported  The mandated minimum floating-point capability is CL_FP_ROUND_TO_NEAREST | CL_FP_INF_NAN. */
    //KEY(CL_DEVICE_TYPE                          , cl_device_type)
    /* CL_DEVICE_TYPE_CPU, CL_DEVICE_TYPE_GPU, CL_DEVICE_TYPE_ACCELERATOR, or CL_DEVICE_TYPE_DEFAULT. */
  KEY(CL_DEVICE_VENDOR                        , cl_string)
  KEY(CL_DEVICE_VENDOR_ID                     , cl_uint)
  KEY(CL_DEVICE_VERSION                       , cl_string)
    //  this name doesn't fit the naming convention KEY(CL_DRIVER_VERSION                       , cl_string)
  assert(0);
#undef KEY
}

CAMLprim value caml_opencl_device_info(value _id, value var)
{
  CAMLparam2(_id, var);
  CAMLlocal1(ans);

  cl_device_id id = Device_id_val(_id);
  int len = 1024;
  char buf[len];
  postprocess_fn postprocess;
  size_t bufLen;

  check_err(clGetDeviceInfo(id, Device_info(var, &postprocess), len, buf, &bufLen));
  CAMLreturn(postprocess(buf, bufLen));
}

/*
static cl_context_properties Context_properties_val(value var)
{
  if (var == hash_variant("Platform"))
    return CL_CONTEXT_PLATFORM;
  else
    assert(0);
}
*/

static cl_device_type Device_type_val(value var)
{
  if (var == hash_variant("CPU"))
    return CL_DEVICE_TYPE_CPU;
  else if (var == hash_variant("GPU"))
    return CL_DEVICE_TYPE_GPU;
  else if (var == hash_variant("Accelerator"))
    return CL_DEVICE_TYPE_ACCELERATOR;
  else if (var == hash_variant("Default"))
    return CL_DEVICE_TYPE_DEFAULT;
  else if (var == hash_variant("All"))
    return CL_DEVICE_TYPE_ALL;
  else
    assert(0);
}

#define Context_val(v) *((cl_context*)Data_custom_val(v))

static void context_finalize(value context)
{
  check_err(clReleaseContext(Context_val(context)));
}

static struct custom_operations context_ops = {
  "caml_opencl_context",
  context_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value caml_opencl_create_context_from_type(value platform, value device_type)
{
  CAMLparam2(platform, device_type);
  CAMLlocal1(ans);
  cl_context_properties *cprops = NULL;
  cl_context context;
  cl_int err;

  if (Is_block(platform))
    {
      cprops = malloc(3*sizeof(cl_context_properties));
      cprops[0] = CL_CONTEXT_PLATFORM;
      cprops[1] = (cl_context_properties)(Platform_id_val(Field(platform,0)));
      cprops[2] = 0;
    }

  context = clCreateContextFromType(cprops, Device_type_val(device_type), NULL, NULL, &err);
  free(cprops);
  check_err(err);
  assert(context);
  ans = alloc_custom(&context_ops, sizeof(cl_context), 0, 1);
  Context_val(ans) = context;

  CAMLreturn(ans);
}

#define Val_device_id(d) (value)d
#define Device_id_val(v) (cl_device_id)v

CAMLprim value caml_opencl_context_devices(value context)
{
  CAMLparam1(context);
  CAMLlocal1(ans);
  size_t ndev;
  cl_device_id *devs;
  int i;

  check_err(clGetContextInfo(Context_val(context), CL_CONTEXT_DEVICES, 0, NULL, &ndev));
  devs = malloc(ndev * sizeof(cl_device_id));
  check_err_free(clGetContextInfo(Context_val(context), CL_CONTEXT_DEVICES, ndev, devs, NULL), devs);
  ans = caml_alloc_tuple(ndev);
  for (i = 0; i < ndev; i++)
    Store_field(ans, i, Val_device_id(devs[i]));
  free(devs);

  CAMLreturn(ans);
}

#define Program_val(v) *((cl_program*)Data_custom_val(v))

static void program_finalize(value prog)
{
  check_err(clReleaseProgram(Program_val(prog)));
}

static struct custom_operations program_ops = {
  "caml_opencl_program",
  program_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value caml_opencl_create_program_with_source(value context, value source)
{
  CAMLparam2(context, source);
  CAMLlocal1(ans);
  cl_program prog;
  const char *s;
  size_t len;
  cl_int err;

  len = caml_string_length(source);
  s = String_val(source);
  prog = clCreateProgramWithSource(Context_val(context), 1, &s, &len, &err);
  check_err(err);
  assert(prog);
  ans = alloc_custom(&program_ops, sizeof(cl_program), 0, 1);
  Program_val(ans) = prog;

  CAMLreturn(ans);
}

CAMLprim value caml_opencl_build_program(value prog, value devices, value options)
{
  CAMLparam3(prog, devices, options);
  cl_uint ndevs = 0;
  cl_device_id *devs = NULL;
  cl_int err;
  int i;

  if (Is_block(devices))
    {
      ndevs = Wosize_val(Field(devices, 0));
      devs = malloc(ndevs * sizeof(cl_device_id));
      for (i = 0; i < ndevs; i++)
        devs[i] = Device_id_val(Field(Field(devices, 0), i));
    }

  err = clBuildProgram(Program_val(prog), ndevs, devs, String_val(options), NULL, NULL);
  if (devs) free(devs);
  check_err(err);

  CAMLreturn(Val_unit);
}

CAMLprim value caml_opencl_program_build_log(value prog, value device)
{
  CAMLparam2(prog, device);
  CAMLlocal1(ans);
  size_t len;
  char *log;

  check_err(clGetProgramBuildInfo(Program_val(prog), Device_id_val(device), CL_PROGRAM_BUILD_LOG, 0, NULL, &len));
  log = malloc(len);
  check_err_free(clGetProgramBuildInfo(Program_val(prog), Device_id_val(device), CL_PROGRAM_BUILD_LOG, len, log, NULL), log);

  ans = caml_copy_string(log);
  free(log);

  CAMLreturn(ans);
}

typedef struct
{
  cl_mem m;
  value v;  // The corresponding bigarray is global rooted
} mem_t;

#define Mem_t_val(v) (*((mem_t**)Data_custom_val(v)))
#define Mem_val(v) Mem_t_val(v)->m

static void mem_finalize(value mem)
{
  check_err(clReleaseMemObject(Mem_val(mem)));
}

static void mem_really_finalize(cl_mem memobj, void *user_data)
{
  mem_t *mem = user_data;
  if (mem->v)
    caml_remove_global_root(&mem->v);
  free(mem);
}

static struct custom_operations mem_ops = {
  "caml_opencl_mem",
  mem_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value caml_opencl_create_buffer(value context, value flags, value _buf)
{
  CAMLparam3(context, flags, _buf);
  CAMLlocal1(ans);
  cl_mem m;
  cl_mem_flags mf;
  cl_int err;
  int i;
  mem_t *mem = malloc(sizeof(mem_t));
  mem->v = 0;
  void *buf;

  buf = Caml_ba_data_val(_buf);

  mf = 0;
  for (i = 0; i < Wosize_val(flags); i++)
    {
      if (Field(flags, i) == hash_variant("Read_write"))
	mf |= CL_MEM_READ_WRITE;
      else if (Field(flags, i) == hash_variant("Write_only"))
	mf |= CL_MEM_WRITE_ONLY;
      else if (Field(flags, i) == hash_variant("Read_only"))
	mf |= CL_MEM_READ_ONLY;
      else if (Field(flags, i) == hash_variant("Alloc"))
	buf = NULL;
    }
  /* We always use host pointers, is it a good idea? */
  if (buf)
    mf |= CL_MEM_USE_HOST_PTR;

  m = clCreateBuffer(Context_val(context), mf, caml_ba_byte_size(Caml_ba_array_val(_buf)), buf, &err);
  check_err(err);
  assert(m);

  mem->m = m;
  if (buf)
    {
      mem->v = _buf;
      // buf should stay alive while m is
      caml_register_global_root(&mem->v);
    }
  /* TODO: find a way to cleanly handle memory in 1.0 */
#ifdef CL_VERSION_1_1
  check_err(clSetMemObjectDestructorCallback(m, mem_really_finalize, mem));
#endif
  ans = alloc_custom(&mem_ops, sizeof(mem_t*), 0, 1);
  Mem_t_val(ans) = mem;

  CAMLreturn(ans);
}

#define Kernel_val(v) *((cl_kernel*)Data_custom_val(v))

static void kernel_finalize(value kernel)
{
  check_err(clReleaseKernel(Kernel_val(kernel)));
}

static struct custom_operations kernel_ops = {
  "caml_opencl_kernel",
  kernel_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value caml_opencl_create_kernel(value prog, value name)
{
  CAMLparam2(prog, name);
  CAMLlocal1(ans);
  cl_kernel kernel;
  cl_int err;

  kernel = clCreateKernel(Program_val(prog), String_val(name), &err);
  check_err(err);
  assert(kernel);
  ans = alloc_custom(&kernel_ops, sizeof(cl_kernel), 0, 1);
  Kernel_val(ans) = kernel;

  CAMLreturn(ans);
}

CAMLprim value caml_opencl_set_kernel_arg_buffer(value kernel, value index, value buffer)
{
  CAMLparam3(kernel, index, buffer);
  cl_mem m = Mem_val(buffer);

  check_err(clSetKernelArg(Kernel_val(kernel), Int_val(index), sizeof(cl_mem), &m));

  CAMLreturn(Val_unit);
}

CAMLprim value caml_opencl_set_kernel_arg_int(value kernel, value index, value _n)
{
  CAMLparam3(kernel, index, _n);
  int n = Int_val(_n);

  check_err(clSetKernelArg(Kernel_val(kernel), Int_val(index), sizeof(int), &n));

  CAMLreturn(Val_unit);
}

#define Event_val(v) *((cl_event*)Data_custom_val(v))

static void event_finalize(value event)
{
  check_err(clReleaseEvent(Event_val(event)));
}

static struct custom_operations event_ops = {
  "caml_opencl_event",
  event_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value caml_opencl_wait_for_event(value event)
{
  CAMLparam1(event);
  cl_event e = Event_val(event);

  check_err(clWaitForEvents(1, &e));

  CAMLreturn(Val_unit);
}

CAMLprim value caml_opencl_event_profiling_info(value event, value param)
{
  CAMLparam2(event, param);
  cl_ulong t;
  cl_profiling_info p;

  if (param == hash_variant("Command_queued"))
    p = CL_PROFILING_COMMAND_QUEUED;
  else if (param == hash_variant("Command_submit"))
    p = CL_PROFILING_COMMAND_SUBMIT;
  else if (param == hash_variant("Command_start"))
    p = CL_PROFILING_COMMAND_START;
  else if (param == hash_variant("Command_end"))
    p = CL_PROFILING_COMMAND_END;
  else
    assert(0);

  check_err(clGetEventProfilingInfo(Event_val(event), p, sizeof(cl_ulong), &t, NULL));

  CAMLreturn(caml_copy_int64(t));
}

#define Command_queue_val(v) *((cl_command_queue*)Data_custom_val(v))

static void command_queue_finalize(value queue)
{
  check_err(clReleaseCommandQueue(Command_queue_val(queue)));
}

static struct custom_operations command_queue_ops = {
  "caml_opencl_command_queue",
  command_queue_finalize,
  custom_compare_default,
  custom_hash_default,
  custom_serialize_default,
  custom_deserialize_default
};

CAMLprim value caml_opencl_create_command_queue(value context, value device)
{
  CAMLparam2(context, device);
  CAMLlocal1(ans);
  cl_command_queue queue;
  cl_int err;

  /* TODO: properties */
  queue = clCreateCommandQueue(Context_val(context), Device_id_val(device), 0, &err);
  check_err(err);
  assert(queue);
  ans = alloc_custom(&command_queue_ops, sizeof(cl_command_queue), 0, 1);
  Command_queue_val(ans) = queue;

  CAMLreturn(ans);
}

CAMLprim value caml_opencl_finish(value queue)
{
  CAMLparam1(queue);

  check_err(clFinish(Command_queue_val(queue)));

  CAMLreturn(Val_unit);
}

CAMLprim value caml_opencl_enqueue_nd_range_kernel(value queue, value kernel, value local_work_size, value global_work_size)
{
  CAMLparam4(queue, kernel, local_work_size, global_work_size);
  CAMLlocal1(ans);

  int work_dim = Wosize_val(global_work_size);
  size_t gws[work_dim];
  size_t *lws = NULL;
  int i;
  cl_event e;

  if (Is_block(local_work_size))
    {
      assert(Wosize_val(Field(local_work_size, 0)) == Wosize_val(global_work_size));
      lws = malloc(work_dim * sizeof(size_t));
    }

  for (i = 0; i < work_dim; i++)
    {
      gws[i] = Int_val(Field(global_work_size, i));
      if (lws)
        lws[i] = Int_val(Field(Field(local_work_size, 0), i));
    }

  check_err_free(clEnqueueNDRangeKernel(Command_queue_val(queue), Kernel_val(kernel), work_dim, NULL, gws, lws, 0, NULL, &e), lws);
  if (lws) free(lws);
  ans = alloc_custom(&event_ops, sizeof(cl_event), 0, 1);
  Event_val(ans) = e;

  CAMLreturn(ans);
}

CAMLprim value caml_opencl_enqueue_read_buffer(value queue, value buffer, value blocking, value offset, value ba)
{
  CAMLparam5(queue, buffer, blocking, offset, ba);
  CAMLlocal1(ans);

  cl_event e;

  check_err(clEnqueueReadBuffer(Command_queue_val(queue), Mem_val(buffer), Bool_val(blocking), Int_val(offset), caml_ba_byte_size(Caml_ba_array_val(ba)), Caml_ba_data_val(ba), 0, NULL, &e));

  ans = alloc_custom(&event_ops, sizeof(cl_event), 0, 1);
  Event_val(ans) = e;

  CAMLreturn(ans);
}

/*
CAMLprim value caml_opencl_equeue_write_buffer(value queue, value buffer, value blocking, value offset)
{
  CAMLparam4(queue, buffer, blocking, offset);
  CAMLlocal1(ans);

  cl_event e;

  check_err(clEnqueueWriteBuffer(Command_queue_val(queue), Mem_val(buffer), Int_val(blocking), Int_val(offset), NULL,));

  ans = alloc_custom(&event_ops, sizeof(cl_event), 0, 1);
  Event_val(ans) = e;

  CAMLreturn(ans);
}
*/

CAMLprim value caml_opencl_unload_compiler(value unit)
{
  CAMLparam0();

  check_err(clUnloadCompiler());

  CAMLreturn(Val_unit);
}
