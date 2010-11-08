exception Error of int

let init () =
  Callback.register_exception "opencl_exn_error" (Error 0)

module Platform = struct
  type t

  external available : unit -> t array = "caml_opencl_platform_ids"

  type info = [`Profile | `Version | `Name | `Vendor | `Extensions]

  external info : t -> info -> string = "caml_opencl_platform_info"

  let profile p = info p `Profile
  let version p = info p `Version
  let name p = info p `Name
  let vendor p = info p `Vendor
  let extensions p = info p `Extensions
end

module Device = struct
(* type context_properties = [`Platform] *)

  type device_type = [`CPU | `GPU | `Accelerator | `Default | `All]

  type t
end

module Context = struct
  type t

  external create_from_type : Platform.t -> Device.device_type -> t = "caml_opencl_create_context_from_type"

  external devices : t -> Device.t array = "caml_opencl_context_devices"
end

module Command_queue = struct
  type t

  external create : Context.t -> Device.t -> t = "caml_opencl_create_command_queue"
end

module Program = struct
  type t

  external create_with_source : Context.t -> string -> t = "caml_opencl_create_program_with_source"

  external build : t -> Device.t array -> string -> unit = "caml_opencl_build_program"

  external build_log : t -> Device.t -> string = "caml_opencl_program_build_log"
end

module Kernel = struct
  type t

  external create : Program.t -> string -> t = "caml_opencl_create_kernel"

  external release : t -> unit = "caml_opencl_release_kernel"
end
