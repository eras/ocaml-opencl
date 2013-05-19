(** Error executing a function. *)
exception Error of int

(** Initialize the openCL library. This function should be called once, before
    using any other function. *)
val init : unit -> unit

val unload_compiler : unit -> unit

(** Operations on platforms. *)
module Platform : sig
  (** A platform. *)
  type t

  (** List available platforms. *)
  val available : unit -> t array

  val profile : t -> string

  val version : t -> string

  val name : t -> string

  val vendor : t -> string

  val extensions : t -> string list
end

module Device : sig
  type device_type = [ `Accelerator | `All | `CPU | `Default | `GPU ]

  type t

  val address_bits		    : t -> int
  val compiler_available	    : t -> bool
  (* val double_fp_config	    : t -> cl_device_fp_config *)
  val endian_little		    : t -> bool
  val error_correction_support	    : t -> bool
  (* val execution_capabilities	    : t -> cl_device_exec_capabilities *)
  val extensions		    : t -> string list
  val global_mem_cache_size	    : t -> int64
  (* val global_mem_cache_type	    : t -> cl_device_mem_cache_type *)
  val global_mem_cacheline_size	    : t -> int
  val global_mem_size		    : t -> int64
  (* val half_fp_config		    : t -> cl_device_fp_config *)
  val image_support		    : t -> bool
  val image2d_max_height	    : t -> int
  val image2d_max_width		    : t -> int
  val image3d_max_depth		    : t -> int
  val image3d_max_height	    : t -> int
  val image3d_max_width		    : t -> int
  val local_mem_size		    : t -> int64
  (* val local_mem_type		    : t -> cl_device_local_mem_type *)
  val max_clock_frequency	    : t -> int
  val max_compute_units		    : t -> int
  val max_constant_args		    : t -> int
  val max_constant_buffer_size	    : t -> int64
  val max_mem_alloc_size	    : t -> int64
  val max_parameter_size	    : t -> int
  val max_read_image_args	    : t -> int
  val max_samplers		    : t -> int
  val max_work_group_size	    : t -> int
  val max_work_item_dimensions	    : t -> int
  val max_work_item_sizes	    : t -> int
  val max_write_image_args	    : t -> int
  val mem_base_addr_align	    : t -> int
  val min_data_type_align_size	    : t -> int
  val name			    : t -> string
  val platform			    : t -> Platform.t
  val preferred_vector_width_char   : t -> int
  val preferred_vector_width_short  : t -> int
  val preferred_vector_width_int    : t -> int
  val preferred_vector_width_long   : t -> int
  val preferred_vector_width_float  : t -> int
  val preferred_vector_width_double : t -> int
  val profile			    : t -> string
  val profiling_timer_resolution    : t -> int
  (* val queue_properties	    : t -> cl_command_queue_properties *)
  (* val single_fp_config	    : t -> cl_device_fp_config *)
  (* val type			    : t -> cl_t *)
  val vendor			    : t -> string
  val vendor_id			    : t -> int
  val version			    : t -> string
end

module Context : sig
  type t

  val create_from_type : ?platform:Platform.t -> Device.device_type -> t

  (** List devices available on a platform. *)
  val devices : t -> Device.t array
end

module Program : sig
  type t

  val create_with_source_file : Context.t -> string -> t

  val create_with_source : Context.t -> string -> t

  val build : ?options:string -> ?devices:Device.t array -> t -> unit

  val build_log : t -> Device.t -> string
end

module Buffer : sig
  type t

  type flag = [ `Read_only | `Read_write | `Write_only | `Alloc ]

  val create : Context.t -> flag list -> ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t -> t
end

module Kernel : sig
  type t

  val create : Program.t -> string -> t

  type argument = [ `Buffer of Buffer.t | `Int of int | `Local of int ]

  val set_args : t -> argument array -> unit

  val set_arg_int : t -> int -> int -> unit

  val set_arg_buffer : t -> int -> Buffer.t -> unit

  val set_arg_local : t -> int -> int -> unit
end

module Event : sig
  type t

  (** Wait for an event to be completed. *)
  val wait : t -> unit

  val duration : t -> Int64.t
end

module Command_queue : sig
  type t

  val create : Context.t -> Device.t -> t

  val finish : t -> unit

  val nd_range_kernel : t -> Kernel.t -> ?local_work_size:int array -> int array -> Event.t

  val read_buffer : t -> Buffer.t -> bool -> int -> ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t -> Event.t

  val write_buffer : t -> Buffer.t -> bool -> int -> ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t -> Event.t
end

(** Helper function to quickly test a kernel. *)
val run : ?platform:Platform.t -> ?device_type:Device.device_type -> string -> ?build_options:string ->
  string ->
  [ `Buffer_in of ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t
  | `Buffer_out of ('a, 'b, Bigarray.c_layout) Bigarray.Array1.t
  | `Int of int ] array ->
  ?local_work_size:int array -> int array -> unit
