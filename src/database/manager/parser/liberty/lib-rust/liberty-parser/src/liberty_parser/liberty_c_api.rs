use crate::liberty_parser::liberty_data;

use std::ffi::CString;
use std::ops::Deref;
use std::ops::DerefMut;
use std::os::raw::c_char;
use std::os::raw::c_double;
use std::os::raw::c_void;

use super::liberty_data::LibertyAttrValue;

#[repr(C)]
pub struct RustVec {
    data: *mut c_void,
    len: usize,
    cap: usize,
}

fn rust_vec_to_c_array<T>(vec: &Vec<T>) -> RustVec {
    RustVec { data: vec.as_ptr() as *mut c_void, len: vec.len(), cap: vec.capacity() }
}

#[no_mangle]
pub extern "C" fn rust_vec_len(vec: &RustVec) -> usize {
    vec.len
}

// More functions to manipulate the Vec...

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
pub struct RustLibertyGroupStmt {
    file_name: *mut c_char,
    line_no: u32,
    group_name: *mut c_char,
    attri_values: RustVec,
    stmts: RustVec,
}

#[no_mangle]
pub extern "C" fn rust_convert_group_stmt(
    group_stmt: *mut liberty_data::LibertyGroupStmt,
) -> *mut RustLibertyGroupStmt {
    unsafe {
        let file_name_str = (*group_stmt).get_attri().get_file_name();
        let file_name = string_to_c_char(file_name_str);
        let line_no = (*group_stmt).get_attri().get_line_no();
        let group_name_str = (*group_stmt).get_group_name();
        let group_name = string_to_c_char(group_name_str);
        let attri_values_rust_vec = (*group_stmt).get_attri_values();
        let stmts_rust_vec = (*group_stmt).get_stmts();

        let attri_values = rust_vec_to_c_array(attri_values_rust_vec);
        let stmts = rust_vec_to_c_array(stmts_rust_vec);

        let lib_group_stmt = RustLibertyGroupStmt { file_name, line_no, group_name, attri_values, stmts };
        let lib_group_stmt_pointer = Box::new(lib_group_stmt);
        let raw_pointer = Box::into_raw(lib_group_stmt_pointer);
        raw_pointer
    }
}

#[repr(C)]
pub struct RustLibertySimpleAttrStmt {
    file_name: *mut c_char,
    line_no: u32,
    attri_name: *mut c_char,
    attri_value: *const c_void,
}

#[no_mangle]
pub extern "C" fn rust_convert_simple_attribute_stmt(
    simple_attri_stmt: *mut liberty_data::LibertySimpleAttrStmt,
) -> *mut RustLibertySimpleAttrStmt {
    unsafe {
        let file_name_str = (*simple_attri_stmt).get_attri().get_file_name();
        let file_name = string_to_c_char(file_name_str);
        let line_no = (*simple_attri_stmt).get_attri().get_line_no();

        let attri_name_str = (*simple_attri_stmt).get_attri_name();
        let attri_name = string_to_c_char(attri_name_str);

        let attri_value_box = (*simple_attri_stmt).get_attri_value();
        let attri_value = attri_value_box.deref() as *const dyn liberty_data::LibertyAttrValue as *const c_void;

        let lib_simple_attri_stmt = RustLibertySimpleAttrStmt { file_name, line_no, attri_name, attri_value };

        let lib_simple_attri_stmt_pointer = Box::new(lib_simple_attri_stmt);
        let raw_pointer = Box::into_raw(lib_simple_attri_stmt_pointer);
        raw_pointer
    }
}

#[repr(C)]
pub struct RustLibertyComplexAttrStmt {
    file_name: *mut c_char,
    line_no: u32,
    attri_name: *mut c_char,
    attri_values: RustVec,
}

#[no_mangle]
pub extern "C" fn rust_convert_complex_attribute_stmt(
    complex_attri_stmt: *mut liberty_data::LibertyComplexAttrStmt,
) -> *mut RustLibertyComplexAttrStmt {
    unsafe {
        let file_name_str = (*complex_attri_stmt).get_attri().get_file_name();
        let file_name = string_to_c_char(file_name_str);
        let line_no = (*complex_attri_stmt).get_attri().get_line_no();

        let attri_name_str = (*complex_attri_stmt).get_attri_name();
        let attri_name = string_to_c_char(attri_name_str);

        let attri_values_rust_vec = (*complex_attri_stmt).get_attri_values();
        let attri_values = rust_vec_to_c_array(attri_values_rust_vec);

        let lib_complex_attri_stmt = RustLibertyComplexAttrStmt { file_name, line_no, attri_name, attri_values };

        let lib_complex_attri_stmt_pointer = Box::new(lib_complex_attri_stmt);
        let raw_pointer = Box::into_raw(lib_complex_attri_stmt_pointer);
        raw_pointer
    }
}

#[repr(C)]
pub struct RustLibertyStringValue {
    value: *mut c_char,
}

pub extern "C" fn rust_convert_string_value(
    string_value: *mut liberty_data::LibertyStringValue,
) -> *mut RustLibertyStringValue {
    unsafe {
        let rust_value = (*string_value).get_string_value();
        let value = string_to_c_char(rust_value);

        let lib_value = RustLibertyStringValue { value };

        let lib_value_pointer = Box::new(lib_value);
        let raw_pointer = Box::into_raw(lib_value_pointer);
        raw_pointer
    }
}

#[repr(C)]
pub struct RustLibertyFloatValue {
    value: c_double,
}

pub extern "C" fn rust_convert_float_value(
    float_value: *mut liberty_data::LibertyFloatValue,
) -> *mut RustLibertyFloatValue {
    unsafe {
        let value = (*float_value).get_float_value();

        let lib_value = RustLibertyFloatValue { value };

        let lib_value_pointer = Box::new(lib_value);
        let raw_pointer = Box::into_raw(lib_value_pointer);
        raw_pointer
    }
}
