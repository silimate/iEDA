use crate::vcd_parser::parse_vcd_file;
use crate::vcd_parser::vcd_data;

use std::borrow::BorrowMut;
use std::cell::RefCell;
use std::ffi::CString;
use std::ops::Deref;
use std::ops::DerefMut;
use std::os::raw::*;
use std::ptr::null_mut;
use std::rc::Rc;

use super::vcd_data::VCDScope;
use crate::vcd_parser::vcd_calc_tc_sp::FindScopeClosure;

#[repr(C)]
pub struct RustVec {
    data: *mut c_void,
    len: usize,
    cap: usize,
    type_size: usize,
}

fn rust_vec_to_c_array<T>(vec: &Vec<T>) -> RustVec {
    RustVec {
        data: vec.as_ptr() as *mut c_void,
        len: vec.len(),
        cap: vec.capacity(),
        type_size: std::mem::size_of::<T>(),
    }
}

#[no_mangle]
pub extern "C" fn rust_vec_len(vec: &RustVec) -> usize {
    vec.len
}

fn string_to_c_char(s: &str) -> *mut c_char {
    let cs = CString::new(s).unwrap();
    cs.into_raw()
}

#[no_mangle]
pub extern "C" fn free_c_char(s: *mut c_char) {
    unsafe {
        let _ = CString::from_raw(s);
    }
}

#[repr(C)]
pub struct RustVCDSignal {
    hash: *mut c_char,
    name: *mut c_char,
    bus_index: *mut c_void,
    signal_size: c_uint,
    signal_type: c_uint, //convert type to number
    scope: *mut c_void,
}

#[repr(C)]
pub struct RustVCDScope {
    name: *mut c_char,
    parent_scope: *mut c_void,
    children_scope: RustVec,
    scope_signals: RustVec,
}

#[repr(C)]
pub struct RustVCDFile {
    start_time: c_longlong,
    end_time: c_longlong,
    time_resolution: c_uint,
    time_unit: c_uint, //convert time unit to number 0 to 5
    date: *mut c_char,
    version: *mut c_char,
    comment: *mut c_char,
    scope_root: *mut c_void,
}

/*structs to calc toggle and sp */
#[repr(C)]
pub struct RustSignalTC {
    signal_name: *mut c_char,
    signal_tc: u64,
}

#[repr(C)]
pub struct RustSignalDuration {
    signal_name: *mut c_char,
    bit_0_duration: u64,
    bit_1_duration: u64,
    bit_x_duration: u64,
    bit_z_duration: u64,
}

#[repr(C)]
pub struct Indexes {
    lindex: i32,
    rindex: i32,
}
#[no_mangle]
pub extern "C" fn rust_convert_signal_index(bus_index: *mut c_void) -> *mut Indexes {
    let bus_index_ptr = bus_index as *mut (i32, i32);

    let bus_index_value = unsafe { *bus_index_ptr };

    let indexes = Indexes {
        lindex: bus_index_value.0,
        rindex: bus_index_value.1,
    };

    let indexes_pointer = Box::new(indexes);
    let raw_pointer = Box::into_raw(indexes_pointer);
    raw_pointer
}

#[no_mangle]
pub extern "C" fn rust_convert_vcd_signal(
    c_vcd_signal: *const vcd_data::VCDSignal,
) -> *mut RustVCDSignal {
    unsafe {
        let signal_hash = (*c_vcd_signal).get_hash();
        let hash = string_to_c_char(signal_hash);

        let signal_name = (*c_vcd_signal).get_name();
        let name = string_to_c_char(signal_name);

        let bus_index_option = (*c_vcd_signal).get_bus_index().clone();

        let bus_index_ptr = match bus_index_option {
            Some(bus_index) => &bus_index as *const (i32, i32) as *mut c_void,
            None => null_mut(),
        };

        let signal_size = (*c_vcd_signal).get_signal_size();

        let signal_type_c = (*c_vcd_signal).get_signal_type();
        let signal_type = (*signal_type_c) as u32;

        let scope_option = (*c_vcd_signal).get_scope().clone();
        let mut scope_ptr = match scope_option {
            Some(scope) => {
                let scope_ptr = scope.deref().as_ptr();
                scope_ptr as *mut c_void
            }
            None => null_mut(),
        };

        let vcd_signal = RustVCDSignal {
            hash,
            name,
            bus_index: bus_index_ptr,
            signal_size,
            signal_type,
            scope: scope_ptr,
        };

        let vcd_signal_pointer = Box::new(vcd_signal);
        let raw_pointer = Box::into_raw(vcd_signal_pointer);
        raw_pointer
    }
}

#[no_mangle]
pub extern "C" fn rust_convert_vcd_scope(
    c_vcd_scope: *const vcd_data::VCDScope,
) -> *mut RustVCDScope {
    unsafe {
        let scope_name = (*c_vcd_scope).get_name();
        let name = string_to_c_char(scope_name);

        let parent_scope_option = (*c_vcd_scope).get_parent_scope().clone();
        let mut parent_scope_ptr = match parent_scope_option {
            Some(parent_scope) => {
                let scope_ptr = parent_scope.deref().as_ptr();
                scope_ptr as *mut c_void
            }
            None => null_mut(),
        };

        let the_scope_children_scope = (*c_vcd_scope).get_children_scopes();
        let children_scope = rust_vec_to_c_array(the_scope_children_scope);

        let the_scope_signals = (*c_vcd_scope).get_scope_signals();

        let scope_signals = rust_vec_to_c_array(the_scope_signals);

        let vcd_scope = RustVCDScope {
            name,
            parent_scope: parent_scope_ptr,
            children_scope,
            scope_signals,
        };
        let vcd_scope_pointer = Box::new(vcd_scope);
        let raw_pointer = Box::into_raw(vcd_scope_pointer);
        raw_pointer
    }
}

#[no_mangle]
pub extern "C" fn rust_convert_vcd_file(c_vcd_file: *mut vcd_data::VCDFile) -> *mut RustVCDFile {
    unsafe {
        let start_time = (*c_vcd_file).get_start_time();
        let end_time = (*c_vcd_file).get_end_time();
        let time_resolution = (*c_vcd_file).get_time_resolution();

        let time_unit_c = (*c_vcd_file).get_time_unit();

        let time_unit = *time_unit_c as u32;

        let vcd_date = (*c_vcd_file).get_date();
        let date = string_to_c_char(vcd_date);

        let vcd_version = (*c_vcd_file).get_version();
        let version = string_to_c_char(vcd_version);

        let vcd_comment = (*c_vcd_file).get_comment();
        let comment = string_to_c_char(vcd_comment);

        let vcd_root_scope = (*c_vcd_file).get_root_scope();

        let root_scope = match vcd_root_scope {
            Some(the_scope) => {
                let scope_ptr = the_scope.deref().as_ptr();
                scope_ptr as *mut c_void
            }
            None => null_mut(),
        };

        let void_ptr = root_scope as *mut VCDScope;

        let scope_root = rust_convert_vcd_scope(void_ptr);

        let vcd_file = RustVCDFile {
            start_time,
            end_time,
            time_resolution,
            time_unit,
            date,
            version,
            comment,
            scope_root: scope_root as *mut c_void,
        };

        let vcd_file_pointer = Box::new(vcd_file);
        let raw_pointer = Box::into_raw(vcd_file_pointer);
        raw_pointer
    }
}

#[no_mangle]
pub extern "C" fn rust_parse_vcd(lib_path: *const c_char) -> *mut c_void {
    let c_str = unsafe { std::ffi::CStr::from_ptr(lib_path) };
    let r_str = c_str.to_string_lossy().into_owned();
    println!("r str {}", r_str);

    let vcd_file = parse_vcd_file(&r_str);

    let vcd_file_pointer = Box::new(vcd_file.unwrap());

    let raw_pointer = Box::into_raw(vcd_file_pointer);
    raw_pointer as *mut c_void
}

struct TraverseScopeClosure {
    closure: Box<dyn Fn(&VCDScope)>,
}

impl TraverseScopeClosure {
    fn new() -> Self {
        let closure = Box::new(|parent_scope: &VCDScope| {
            let signals = parent_scope.get_scope_signals();
            // Calculate the signal of the current layer scope
            for scope_signal in signals {
                if let vcd_data::VCDVariableType::VarWire = *scope_signal.get_signal_type() {
                } else {
                    continue;
                }
            }

            // View the next level of the scope
            let children_scopes = parent_scope.get_children_scopes();
            for child_scope in children_scopes {
                let recursive_closure = TraverseScopeClosure::new();
                // (recursive_closure.closure)(child_scope.as_ref().get_mut());
            }
        });

        Self { closure }
    }
}

#[no_mangle]
pub extern "C" fn rust_calc_scope_tc_sp(
    c_top_vcd_scope_name: *const c_char,
    c_vcd_file: *mut vcd_data::VCDFile,
) {
    unsafe {
        let c_str = unsafe { std::ffi::CStr::from_ptr(c_top_vcd_scope_name) };
        let r_str = c_str.to_string_lossy().into_owned();
        println!("r str {}", r_str);

        match (*c_vcd_file).get_root_scope() {
            Some(the_scope) => {
                let find_scope_closure = FindScopeClosure::new(&the_scope, &r_str);
                (find_scope_closure.closure)(&the_scope, &r_str);
            }
            None => panic!("root scope not exist."),
        };

        // (recursive_closure.closure)(&*c_top_vcd_scope);
    }
}
