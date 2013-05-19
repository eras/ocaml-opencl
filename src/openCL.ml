exception Error of int

let init () =
  Callback.register_exception "opencl_exn_error" (Error 0)

external unload_compiler : unit -> unit = "caml_opencl_unload_compiler"

  (* for splitting extension list *)
let split str =
  let rec aux prev cur accu =
    if cur = String.length str
    then (prev, List.rev accu)
    else
      let prev, accu =
	if str.[cur] = ' '
	then (cur + 1, String.sub str prev (cur - prev)::accu)
	else (prev, accu)
      in
      aux prev (cur + 1) accu
  in
  snd (aux 0 0 [])

module Platform = struct
  type t

  external available : unit -> t array = "caml_opencl_platform_ids"

  type info = [`Profile | `Version | `Name | `Vendor | `Extensions]

  external info : t -> info -> string = "caml_opencl_platform_info"

  let profile p = info p `Profile
  let version p = info p `Version
  let name p = info p `Name
  let vendor p = info p `Vendor
  let extensions p = split (info p `Extensions)
end

module Device = struct
(* type context_properties = [`Platform] *)

  type device_type = [`CPU | `GPU | `Accelerator | `Default | `All]

  type t

  type info = [
  | `Address_bits
  | `Compiler_available
  | `Double_fp_config
  | `Endian_little
  | `Error_correction_support
  | `Execution_capabilities
  | `Extensions
  | `Global_mem_cache_size
  | `Global_mem_cache_type
  | `Global_mem_cacheline_size
  | `Global_mem_size
  | `Half_fp_config
  | `Image_support
  | `Image2d_max_height
  | `Image2d_max_width
  | `Image3d_max_depth
  | `Image3d_max_height
  | `Image3d_max_width
  | `Local_mem_size
  | `Local_mem_type
  | `Max_clock_frequency
  | `Max_compute_units
  | `Max_constant_args
  | `Max_constant_buffer_size
  | `Max_mem_alloc_size
  | `Max_parameter_size
  | `Max_read_image_args
  | `Max_samplers
  | `Max_work_group_size
  | `Max_work_item_dimensions
  | `Max_work_item_sizes
  | `Max_write_image_args
  | `Mem_base_addr_align
  | `Min_data_type_align_size
  | `Name
  | `Platform
  | `Preferred_vector_width_char
  | `Preferred_vector_width_short
  | `Preferred_vector_width_int
  | `Preferred_vector_width_long
  | `Preferred_vector_width_float
  | `Preferred_vector_width_double
  | `Profile
  | `Profiling_timer_resolution
  | `Queue_properties
  | `Single_fp_config
  | `Type
  | `Vendor
  | `Vendor_id
  | `Version ]
    
  type cl_uint	      = int
  type cl_bool	      = bool
  type cl_string      = string
  type cl_ulong	      = int64
  type size_t	      = int
  type cl_platform_id = Platform.t

  external info : t -> info -> 'a = "caml_opencl_device_info"

  let address_bits d		      : cl_uint			    = info d `Address_bits
  let compiler_available d	      : cl_bool			    = info d `Compiler_available
  (* let double_fp_config d	      : cl_device_fp_config	    = info d `Double_fp_config *)
  let endian_little d		      : cl_bool			    = info d `Endian_little
  let error_correction_support d      : cl_bool			    = info d `Error_correction_support
  (* let execution_capabilities d     : cl_device_exec_capabilities = info d `Execution_capabilities *)
  let extensions d		      : cl_string list		    = split (info d `Extensions)
  let global_mem_cache_size d	      : cl_ulong		    = info d `Global_mem_cache_size
  (* let global_mem_cache_type d      : cl_device_mem_cache_type    = info d `Global_mem_cache_type *)
  let global_mem_cacheline_size d     : cl_uint			    = info d `Global_mem_cacheline_size
  let global_mem_size d		      : cl_ulong		    = info d `Global_mem_size
  (* let half_fp_config d	      : cl_device_fp_config	    = info d `Half_fp_config *)
  let image_support d		      : cl_bool			    = info d `Image_support
  let image2d_max_height d	      : size_t			    = info d `Image2d_max_height
  let image2d_max_width d	      : size_t			    = info d `Image2d_max_width
  let image3d_max_depth d	      : size_t			    = info d `Image3d_max_depth
  let image3d_max_height d	      : size_t			    = info d `Image3d_max_height
  let image3d_max_width d	      : size_t			    = info d `Image3d_max_width
  let local_mem_size d		      : cl_ulong		    = info d `Local_mem_size
  (* let local_mem_type d	      : cl_device_local_mem_type    = info d `Local_mem_type *)
  let max_clock_frequency d	      : cl_uint			    = info d `Max_clock_frequency
  let max_compute_units d	      : cl_uint			    = info d `Max_compute_units
  let max_constant_args d	      : cl_uint			    = info d `Max_constant_args
  let max_constant_buffer_size d      : cl_ulong		    = info d `Max_constant_buffer_size
  let max_mem_alloc_size d	      : cl_ulong		    = info d `Max_mem_alloc_size
  let max_parameter_size d	      : size_t			    = info d `Max_parameter_size
  let max_read_image_args d	      : cl_uint			    = info d `Max_read_image_args
  let max_samplers d		      : cl_uint			    = info d `Max_samplers
  let max_work_group_size d	      : size_t			    = info d `Max_work_group_size
  let max_work_item_dimensions d      : cl_uint			    = info d `Max_work_item_dimensions
  let max_work_item_sizes d	      : size_t			    = info d `Max_work_item_sizes
  let max_write_image_args d	      : cl_uint			    = info d `Max_write_image_args
  let mem_base_addr_align d	      : cl_uint			    = info d `Mem_base_addr_align
  let min_data_type_align_size d      : cl_uint			    = info d `Min_data_type_align_size
  let name d			      : cl_string		    = info d `Name
  let platform d		      : cl_platform_id		    = info d `Platform
  let preferred_vector_width_char d   : cl_uint			    = info d `Preferred_vector_width_char
  let preferred_vector_width_short d  : cl_uint			    = info d `Preferred_vector_width_short
  let preferred_vector_width_int d    : cl_uint			    = info d `Preferred_vector_width_int
  let preferred_vector_width_long d   : cl_uint			    = info d `Preferred_vector_width_long
  let preferred_vector_width_float d  : cl_uint			    = info d `Preferred_vector_width_float
  let preferred_vector_width_double d : cl_uint			    = info d `Preferred_vector_width_double
  let profile d			      : cl_string		    = info d `Profile
  let profiling_timer_resolution d    : size_t			    = info d `Profiling_timer_resolution
  (* let queue_properties d	      : cl_command_queue_properties = info d `Queue_properties *)
  (* let single_fp_config d	      : cl_device_fp_config	    = info d `Single_fp_config *)
  (* let type d			      : cl_device_type		    = info d `Type *)
  let vendor d			      : cl_string		    = info d `Vendor
  let vendor_id d		      : cl_uint			    = info d `Vendor_id
  let version d			      : cl_string		    = info d `Version
end

module Context = struct
  type t

  external create_from_type : ?platform:Platform.t -> Device.device_type -> t = "caml_opencl_create_context_from_type"

  external devices : t -> Device.t array = "caml_opencl_context_devices"
end

module Program = struct
  type t

  external create_with_source : Context.t -> string -> t = "caml_opencl_create_program_with_source"

  let create_with_source_file c f =
    let src = ref "" in
    let ic = open_in f in
    (
      try
        while true do
          src := !src ^ input_line ic ^ "\n"
        done
      with
        | End_of_file -> ()
	| exn ->
	  close_in ic;
	  raise exn
    );
    close_in ic;
    create_with_source c !src

  external build : t -> ?devices:Device.t array -> string -> unit = "caml_opencl_build_program"
  let build ?(options="") ?devices p = build p ?devices options

  external build_log : t -> Device.t -> string = "caml_opencl_program_build_log"
end

module Buffer = struct
  (* TODO: phantom type to ensure buffer compatibility *)
  type t

  type flag = [ `Read_write | `Read_only | `Write_only | `Alloc ]

  external create : Context.t -> flag array -> ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t -> t = "caml_opencl_create_buffer"
  let create c f b = create c (Array.of_list f) b
end

module Kernel = struct
  type t

  external create : Program.t -> string -> t = "caml_opencl_create_kernel"

  external set_arg_int : t -> int -> int -> unit = "caml_opencl_set_kernel_arg_int"

  external set_arg_buffer : t -> int -> Buffer.t -> unit = "caml_opencl_set_kernel_arg_buffer"

  external set_arg_local : t -> int -> int -> unit = "caml_opencl_set_kernel_arg_local"

  type argument = [ `Buffer of Buffer.t | `Int of int | `Local of int ]

  let set_args k (a:argument array) =
    for i = 0 to Array.length a - 1 do
      match a.(i) with
        | `Buffer b -> set_arg_buffer k i b
        | `Int n -> set_arg_int k i n
        | `Local n -> set_arg_local k i n
    done
end

module Event = struct
  type t

  external wait : t -> unit = "caml_opencl_wait_for_event"

  type profiling_info = [ `Command_queued | `Command_submit | `Command_start | `Command_end ]

  external profiling_info : t -> profiling_info -> Int64.t = "caml_opencl_event_profiling_info"

  let duration e =
    let b = profiling_info e `Command_start in
    let e = profiling_info e `Command_end in
    Int64.sub e b
end

module Command_queue = struct
  type t

  external create : Context.t -> Device.t -> t = "caml_opencl_create_command_queue"

  external finish : t -> unit = "caml_opencl_finish"

  external nd_range_kernel : t -> Kernel.t -> ?local_work_size:(int array) -> int array -> Event.t = "caml_opencl_enqueue_nd_range_kernel"

  external read_buffer : t -> Buffer.t -> bool -> int -> ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t -> Event.t = "caml_opencl_enqueue_read_buffer"

  external write_buffer : t -> Buffer.t -> bool -> int -> ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t -> Event.t = "caml_opencl_enqueue_write_buffer"
end

let run ?platform ?(device_type=`GPU) kernel_file ?build_options kernel_name args ?local_work_size gws =
  let ctxt = Context.create_from_type ?platform device_type in
  let device = (Context.devices ctxt).(0) in
  let queue = Command_queue.create ctxt device in
  let prog = Program.create_with_source_file ctxt kernel_file in
  (
    try
      Program.build prog ~devices:[|device|] ?options:build_options;
    with
      | e ->
        Printf.eprintf "Error while building:\n%s\n%!" (Program.build_log prog device);
        raise e
  );
  let kernel = Kernel.create prog kernel_name in
  let res = ref [] in
  let args =
    Array.map
      (function
        | `Buffer_in b ->
          `Buffer (Buffer.create ctxt [`Read_only] b)
        | `Buffer_out b ->
          let buf = Buffer.create ctxt [`Write_only; `Alloc] b in
          res := (b, buf) :: !res;
          `Buffer buf
        | `Int n as a -> a
      ) args
  in
  Kernel.set_args kernel args;
  Command_queue.finish queue;
  let _ = Command_queue.nd_range_kernel queue kernel ?local_work_size gws in
  List.iter (fun (b, buf) -> ignore (Command_queue.read_buffer queue buf true 0 b)) !res;
  Command_queue.finish queue
