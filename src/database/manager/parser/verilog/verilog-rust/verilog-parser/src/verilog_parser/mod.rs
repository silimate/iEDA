pub mod verilog_c_api;
pub mod verilog_data;

use pest::iterators::Pair;
use pest::Parser;
use pest_derive::Parser;

use std::ffi::c_void;
use std::os::raw::c_char;

#[derive(Parser)]
#[grammar = "verilog_parser/grammar/verilog.pest"]
pub struct VerilogParser;

fn process_module_id(pair: Pair<Rule>) -> Result<&str, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    match pair_clone.as_rule() {
        Rule::module_id => {
            let module_name = pair_clone.as_str();
            Ok(module_name)
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Failed to parse module id".into() },
            pair_clone.as_span(),
        )),
    }
}

fn process_port_or_wire_id(pair: Pair<Rule>) -> Result<Box<dyn verilog_data::VerilogVirtualBaseID>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    match pair_clone.as_rule() {
            Rule::port_or_wire_id => {
                let id = pair_clone.as_str();
                let verilog_id = verilog_data::VerilogID::new(id);
                let verilog_virtual_base_id: Box<dyn verilog_data::VerilogVirtualBaseID> = Box::new(verilog_id);
               
                Ok(verilog_virtual_base_id)

            },
            _ => todo!(),
        }

}

fn process_inner_port_declaration(pair: Pair<Rule>,dcl_type:verilog_data::DclType) -> Result<Box<dyn verilog_data::VerilogVirtualBaseStmt>, pest::error::Error<Rule>>{
    let pair_clone = pair.clone();
    let mut inner_pair = pair.into_inner();
    let port_list_or_bus_slice_pair = inner_pair.next();
    match port_list_or_bus_slice_pair.clone().unwrap().as_rule() {
        Rule::bus_slice => {                   
            let mut decimal_digits_pair = port_list_or_bus_slice_pair.unwrap().into_inner();
            let range_from = decimal_digits_pair.next().unwrap().as_str().parse::<i32>().unwrap();
            let range_to = decimal_digits_pair.next().unwrap().as_str().parse::<i32>().unwrap();
            let range = Some((range_from, range_to));
            let mut verilog_dcl_vec : Vec<Box<verilog_data::VerilogDcl>>= Vec::new();
            let port_list_pair = inner_pair.next();
            for port_pair in port_list_pair.unwrap().into_inner() {
                let dcl_name = port_pair.as_str().to_string(); 
                let verilog_dcl = verilog_data::VerilogDcl::new(0, dcl_type.clone(), &dcl_name, range);
                verilog_dcl_vec.push(Box::new(verilog_dcl.clone()));
            }
            let verilog_dcls = verilog_data::VerilogDcls::new(0, verilog_dcl_vec);
            Ok(Box::new(verilog_dcls) as Box<dyn verilog_data::VerilogVirtualBaseStmt>)
            
        }
        Rule::port_list => {
            let range = None;
            let mut verilog_dcl_vec : Vec<Box<verilog_data::VerilogDcl>>= Vec::new();
            let port_list_pair = port_list_or_bus_slice_pair;
            for port_pair in port_list_pair.unwrap().into_inner() {
                let dcl_name = port_pair.as_str().to_string(); 
                let verilog_dcl = verilog_data::VerilogDcl::new(0, dcl_type.clone(), &dcl_name, range);
                verilog_dcl_vec.push(Box::new(verilog_dcl.clone()));
            }
            let verilog_dcls = verilog_data::VerilogDcls::new(0, verilog_dcl_vec);
            Ok(Box::new(verilog_dcls) as Box<dyn verilog_data::VerilogVirtualBaseStmt>)
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
    }
}

fn process_port_declaration(pair: Pair<Rule>) -> Result<Box<dyn verilog_data::VerilogVirtualBaseStmt>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    match pair.as_rule() {
        Rule::input_declaration => {
            let dcl_type = verilog_data::DclType::KInput;
            let verilog_dcls = process_inner_port_declaration(pair,dcl_type);
            verilog_dcls
        }
        Rule::output_declaration => {
            let dcl_type = verilog_data::DclType::KOutput;
            let verilog_dcls = process_inner_port_declaration(pair,dcl_type);
            verilog_dcls
        }
        Rule::inout_declaration => {
            let dcl_type = verilog_data::DclType::KInout;
            let verilog_dcls = process_inner_port_declaration(pair,dcl_type);
            verilog_dcls
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
    }
}

fn process_inner_wire_declaration(pair: Pair<Rule>,dcl_type:verilog_data::DclType) -> Result<Box<dyn verilog_data::VerilogVirtualBaseStmt>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    let mut inner_pair = pair.into_inner();
    let wire_list_or_bus_slice_pair = inner_pair.next();
    match wire_list_or_bus_slice_pair.clone().unwrap().as_rule() {
        Rule::bus_slice => {                   
            let mut decimal_digits_pair = wire_list_or_bus_slice_pair.unwrap().into_inner();
            let range_from = decimal_digits_pair.next().unwrap().as_str().parse::<i32>().unwrap();
            let range_to = decimal_digits_pair.next().unwrap().as_str().parse::<i32>().unwrap();
            let range = Some((range_from, range_to));
            let mut verilog_dcl_vec : Vec<Box<verilog_data::VerilogDcl>>= Vec::new();
            let wire_list_pair = inner_pair.next();
            for wire_pair in wire_list_pair.unwrap().into_inner() {
                let dcl_name = wire_pair.as_str().to_string(); 
                let verilog_dcl = verilog_data::VerilogDcl::new(0, dcl_type.clone(), &dcl_name, range);
                verilog_dcl_vec.push(Box::new(verilog_dcl.clone()));
            }
            let verilog_dcls = verilog_data::VerilogDcls::new(0, verilog_dcl_vec);
            Ok(Box::new(verilog_dcls) as Box<dyn verilog_data::VerilogVirtualBaseStmt>)
            
        }
        Rule::wire_list => {
            let range = None;
            let mut verilog_dcl_vec : Vec<Box<verilog_data::VerilogDcl>>= Vec::new();
            let wire_list_pair = wire_list_or_bus_slice_pair;
            for wire_pair in wire_list_pair.unwrap().into_inner() {
                let dcl_name = wire_pair.as_str().to_string(); 
                let verilog_dcl = verilog_data::VerilogDcl::new(0, dcl_type.clone(), &dcl_name, range);
                verilog_dcl_vec.push(Box::new(verilog_dcl.clone()));
            }
            let verilog_dcls = verilog_data::VerilogDcls::new(0, verilog_dcl_vec);
            Ok(Box::new(verilog_dcls) as Box<dyn verilog_data::VerilogVirtualBaseStmt>)
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
    }
}

fn process_wire_declaration(pair: Pair<Rule>) -> Result<Box<dyn verilog_data::VerilogVirtualBaseStmt>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    match pair.as_rule() {
        Rule::wire_declaration => {
            let dcl_type = verilog_data::DclType::KWire;
            let verilog_dcls = process_inner_wire_declaration(pair,dcl_type);
            verilog_dcls
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
    }
}

fn extract_range(input: &str) -> Option<(&str, i32, i32)> {
    if let Some(open_bracket) = input.find('[') {
        if let Some(close_bracket) = input.find(']') {
            if let Some(colon) = input.find(':') {
                let name = &input[..open_bracket];
                let start = input[open_bracket + 1..colon].parse().ok()?;
                let end = input[colon + 1..close_bracket].parse().ok()?;
                return Some((name, start, end));
            }
        }
    }
    None
}

fn extract_single(input: &str) -> Option<(&str, i32)> {
    if let Some(open_bracket) = input.find('[') {
        if let Some(close_bracket) = input.find(']') {
            let name = &input[..open_bracket];
            let index = input[open_bracket + 1..close_bracket].parse().ok()?;
            return Some((name, index));
        }
    }
    None
}

fn extract_name(input: &str) -> Option<&str> {
    Some(input)
}

fn build_verilog_virtual_base_id(input: &str) -> Box<dyn verilog_data::VerilogVirtualBaseID> {
    let verilog_virtual_base_id: Box<dyn verilog_data::VerilogVirtualBaseID>;
    if let Some((name, range_from, range_to)) = extract_range(input) {
        let verilog_slice_id = verilog_data::VerilogSliceID::new(name, range_from, range_to);
        verilog_virtual_base_id = Box::new(verilog_slice_id);
    } else if let Some((name, index)) = extract_single(input) {
        let verilog_index_id = verilog_data::VerilogIndexID::new(name, index);
        verilog_virtual_base_id = Box::new(verilog_index_id);
    } else if let Some(name) = extract_name(input) {
        let verilog_id = verilog_data::VerilogID::new(name);       
        verilog_virtual_base_id = Box::new(verilog_id);
    } else {
        let verilog_id = verilog_data::VerilogID::default();
        verilog_virtual_base_id = Box::new(verilog_id);
    }   

    verilog_virtual_base_id
}


fn process_first_port_connection_single_connect(pair: Pair<Rule>)->Result<Box<verilog_data::VerilogPortRefPortConnect>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    let mut inner_pairs = pair.into_inner();
    let length = inner_pairs.clone().count();
    match length {
        2 => {
            let port = inner_pairs.next().unwrap().as_str();
            let port_id = build_verilog_virtual_base_id(port);
            let net_connect_pair = inner_pairs.next().unwrap();
            match net_connect_pair.as_rule() {
                Rule::scalar_constant => {
                    let net_connect = net_connect_pair.as_str();
                    let verilog_id = verilog_data::VerilogID::new(net_connect);
                    let verilog_virtual_base_id: Box<dyn verilog_data::VerilogVirtualBaseID> = Box::new(verilog_id);
                    let verilog_const_net_expr = verilog_data::VerilogConstantExpr::new(0,verilog_virtual_base_id);
                    let net_expr:Box<dyn verilog_data::VerilogVirtualBaseNetExpr> = Box::new(verilog_const_net_expr);
                    let port_ref = Box::new(verilog_data::VerilogPortRefPortConnect::new(port_id, Some(net_expr)));
                    Ok(port_ref)
                }
                Rule::port_or_wire_id => {
                    let net_connect = net_connect_pair.as_str();
                    let verilog_virtual_base_id: Box<dyn verilog_data::VerilogVirtualBaseID> = build_verilog_virtual_base_id(net_connect);
                    let verilog_net_id_expr = verilog_data::VerilogNetIDExpr::new(0,verilog_virtual_base_id);
                    let net_expr:Box<dyn verilog_data::VerilogVirtualBaseNetExpr> =Box::new(verilog_net_id_expr);
                    let port_ref = Box::new(verilog_data::VerilogPortRefPortConnect::new(port_id, Some(net_expr)));
                    Ok(port_ref)
                }
                _ => Err(pest::error::Error::new_from_span(
                    pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
                    pair_clone.as_span(),
                )),
            }
        }
        1 => {
            let port = inner_pairs.next().unwrap().as_str();
            let  port_id = build_verilog_virtual_base_id(port);
            let net_expr: Option<Box<dyn verilog_data::VerilogVirtualBaseNetExpr>> = None;
            let port_ref = Box::new(verilog_data::VerilogPortRefPortConnect::new(port_id, net_expr));
            Ok(port_ref)
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
        }
}


fn process_first_port_connection_multiple_connect(pair: Pair<Rule>)->Result<Box<verilog_data::VerilogPortRefPortConnect>, pest::error::Error<Rule>> {
    let mut inner_pairs = pair.into_inner();
    let port = inner_pairs.next().unwrap().as_str();
    let port_id = build_verilog_virtual_base_id(port);
    let mut verilog_id_concat:Vec<Box<dyn verilog_data::VerilogVirtualBaseNetExpr>> = Vec::new();
    for inner_pair in inner_pairs {
        match inner_pair.as_rule() {
            Rule::scalar_constant => {
                let net_connect = inner_pair.as_str();
                let verilog_id = verilog_data::VerilogID::new(net_connect);
                let verilog_virtual_base_id: Box<dyn verilog_data::VerilogVirtualBaseID> = Box::new(verilog_id);
                let verilog_net_constant_expr = verilog_data::VerilogConstantExpr::new(0,verilog_virtual_base_id);
                let verilog_virtual_base_net_expr: Box<dyn verilog_data::VerilogVirtualBaseNetExpr> = Box::new(verilog_net_constant_expr);
                verilog_id_concat.push(verilog_virtual_base_net_expr);
            }
            Rule::port_or_wire_id => {
                let net_connect = inner_pair.as_str();
                let verilog_virtual_base_id: Box<dyn verilog_data::VerilogVirtualBaseID> = build_verilog_virtual_base_id(net_connect);
                let verilog_net_id_expr = verilog_data::VerilogNetIDExpr::new(0,verilog_virtual_base_id);
                let verilog_virtual_base_net_expr: Box<dyn verilog_data::VerilogVirtualBaseNetExpr> = Box::new(verilog_net_id_expr);
                verilog_id_concat.push(verilog_virtual_base_net_expr);
            }
            _ => unreachable!(),
        }
    }
    let verilog_net_concat_expr = verilog_data::VerilogNetConcatExpr::new(0,verilog_id_concat);
    let net_expr:Box<dyn verilog_data::VerilogVirtualBaseNetExpr> = Box::new(verilog_net_concat_expr);
    let port_ref = Box::new(verilog_data::VerilogPortRefPortConnect::new(port_id, Some(net_expr)));
    Ok(port_ref)
}

fn process_port_block_connection(pair: Pair<Rule>)-> Result<Vec<Box<verilog_data::VerilogPortRefPortConnect>>, pest::error::Error<Rule>> {
    let mut port_connections:Vec<Box<verilog_data::VerilogPortRefPortConnect>> = Vec::new();
    for inner_pair in pair.into_inner() {
        match inner_pair.as_rule() {
            Rule::first_port_connection_single_connect => {
                let port_connection = process_first_port_connection_single_connect(inner_pair);    
                port_connections.push(port_connection.unwrap());
            }
            Rule::first_port_connection_multiple_connect => {
                let port_connection = process_first_port_connection_multiple_connect(inner_pair);
                port_connections.push(port_connection.unwrap());
            }
            // refactor
            _ => unreachable!(),
        }
    }
  Ok(port_connections)
}

fn process_inner_inst_declaration(pair: Pair<Rule>) -> Result<Box<dyn verilog_data::VerilogVirtualBaseStmt>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    let mut inner_pair = pair.into_inner();
    let inst_id_pair = inner_pair.next();
    match inst_id_pair.clone().unwrap().as_rule() {
        Rule::inst_or_cell_id => {                   
            let cell_name = inst_id_pair.unwrap().as_str();
            let inst_name = inner_pair.next().unwrap().as_str();
            let port_connections = process_port_block_connection(inner_pair.next().unwrap());
            let port_connections_vec = port_connections.unwrap();

            let verilog_inst = verilog_data::VerilogInst::new(0, inst_name,cell_name,port_connections_vec);
            Ok(Box::new(verilog_inst) as Box<dyn verilog_data::VerilogVirtualBaseStmt>)
            
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
    }
}

fn process_inst_declaration(pair: Pair<Rule>) -> Result<Box<dyn verilog_data::VerilogVirtualBaseStmt>, pest::error::Error<Rule>> {
    let pair_clone = pair.clone();
    match pair.as_rule() {
        Rule::inst_declaration => {
            let verilog_inst = process_inner_inst_declaration(pair);
            verilog_inst
        }
        _ => Err(pest::error::Error::new_from_span(
            pest::error::ErrorVariant::CustomError { message: "Unknown rule".into() },
            pair_clone.as_span(),
        )),
    }
}

pub fn parse_verilog_file(verilog_file_path: &str) -> Result<verilog_data::VerilogModule, pest::error::Error<Rule>> {
    // Generate verilog.pest parser
    let input_str = std::fs::read_to_string(verilog_file_path).unwrap_or_else(|_| panic!("Can't read file: {}", verilog_file_path));
    let parse_result = VerilogParser::parse(Rule::verilog_file, input_str.as_str());

    let file_name = "tbd";
    let line_no = 0;
    let mut module_name = " ";
    let mut port_list: Vec<Box<dyn verilog_data::VerilogVirtualBaseID>> = Vec::new();
    let mut module_stmts: Vec<Box <dyn verilog_data::VerilogVirtualBaseStmt>> = Vec::new();

    match parse_result {
        Ok(pairs) => {
            // pairs:module_declaration+
            for pair in pairs {
                let inner_pairs = pair.into_inner();
                for inner_pair in inner_pairs {
                    match inner_pair.as_rule() {
                        Rule::module_id => {
                            module_name = process_module_id(inner_pair).unwrap();
                        }
                        Rule::port_list => {
                            for inner_inner_pair in inner_pair.into_inner() {
                                let  port_id = process_port_or_wire_id(inner_inner_pair).unwrap();
                                port_list.push(port_id);
                            }
                        }
                        Rule::port_block_declaration => {
                            for inner_inner_pair in inner_pair.into_inner() {
                                let verilog_dcls =  process_port_declaration(inner_inner_pair).unwrap();
                                module_stmts.push(verilog_dcls);
                            }
                        }
                        Rule::wire_block_declaration => {
                            for inner_inner_pair in inner_pair.into_inner() {
                                let verilog_dcls =  process_wire_declaration(inner_inner_pair).unwrap();
                                module_stmts.push(verilog_dcls);
                            }
                        }
                        Rule::inst_block_declaration => {
                            for inner_inner_pair in inner_pair.into_inner() {
                                let verilog_inst =  process_inst_declaration(inner_inner_pair).unwrap();
                                module_stmts.push(verilog_inst);
                            }
                        }
                        Rule::EOI => (),
                        _ => unreachable!(),
                    }
                }
            }
        }
        Err(err) => {
            // Handle parsing error
            println!("Error: {}", err);
        }
    }

    // store the verilogModule.
    let verilog_module = verilog_data::VerilogModule::new(1, module_name, port_list, module_stmts);
    Ok(verilog_module)
}

// pub fn parse_verilog_file(verilog_file_path: &str) -> Result<Vec<verilog_data::VerilogModule>, pest::error::Error<Rule>> {
//     // Generate verilog.pest parser
//     let input_str = std::fs::read_to_string(verilog_file_path).unwrap_or_else(|_| panic!("Can't read file: {}", verilog_file_path));
//     let parse_result = VerilogParser::parse(Rule::verilog_file, input_str.as_str());

//     let file_name = "tbd";
//     let line_no = 0;
//     let mut verilog_modules: Vec<verilog_data::VerilogModule> = Vec::new();

//     match parse_result {
//         Ok(pairs) => {
//             // pairs:module_declaration+
//             // println!("{:#?}", pairs);
//             for pair in pairs {
//                 let inner_pairs = pair.into_inner();
//                 let mut module_name = " ";
//                 let mut port_list: Vec<Box<dyn verilog_data::VerilogVirtualBaseID>> = Vec::new();
//                 let mut module_stmts: Vec<Box <dyn verilog_data::VerilogVirtualBaseStmt>> = Vec::new();
//                 for inner_pair in inner_pairs {
//                     match inner_pair.as_rule() {
//                         Rule::module_id => {
//                             module_name = process_module_id(inner_pair).unwrap();
//                         }
//                         Rule::port_list => {
//                             for inner_inner_pair in inner_pair.into_inner() {
//                                 let  port_id = process_port_or_wire_id(inner_inner_pair).unwrap();
//                                 port_list.push(port_id);
//                             }
//                         }
//                         Rule::port_block_declaration => {
//                             for inner_inner_pair in inner_pair.into_inner() {
//                                 let verilog_dcls =  process_port_declaration(inner_inner_pair).unwrap();
//                                 module_stmts.push(verilog_dcls);
//                             }
//                         }
//                         Rule::wire_block_declaration => {
//                             for inner_inner_pair in inner_pair.into_inner() {
//                                 let verilog_dcls =  process_wire_declaration(inner_inner_pair).unwrap();
//                                 module_stmts.push(verilog_dcls);
//                             }
//                         }
//                         Rule::inst_block_declaration => {
//                             for inner_inner_pair in inner_pair.into_inner() {
//                                 let verilog_inst =  process_inst_declaration(inner_inner_pair).unwrap();
//                                 module_stmts.push(verilog_inst);
//                             }
//                         }
//                         _ => unreachable!(),
//                     }
//                 }
//                 let verilog_module = verilog_data::VerilogModule::new(1, module_name, port_list, module_stmts);
//                 verilog_modules.push(verilog_module);
//             }
            
//         }
//         Err(err) => {
//             // Handle parsing error
//             println!("Error: {}", err);
//         }
//     }

//     Ok(verilog_modules)
// }

#[no_mangle]
pub extern "C" fn rust_parse_verilog(verilog_path: *const c_char) -> *mut c_void {
    let c_str = unsafe { std::ffi::CStr::from_ptr(verilog_path) };
    let r_str = c_str.to_string_lossy().into_owned();
    println!("r str {}", r_str);

    let verilog_result = parse_verilog_file(&r_str);
    // let verilog_modules:Vec<verilog_data::VerilogModule> = verilog_result.unwrap(); 
    let verilog_module:verilog_data::VerilogModule = verilog_result.unwrap(); 
    let verilog_modules_pointer = Box::new(verilog_module);

    let raw_pointer = Box::into_raw(verilog_modules_pointer);
    raw_pointer as *mut c_void
}

#[no_mangle]
pub extern "C" fn rust_free_verilog_module(c_verilog_module: *mut Vec<verilog_data::VerilogModule>) {
    let _: Box<Vec<verilog_data::VerilogModule>> = unsafe { Box::from_raw(c_verilog_module) };
}



#[cfg(test)]
mod tests {

    use pest::error;
    use pest::iterators::Pair;
    use pest::iterators::Pairs;

    use super::*;

    fn process_pair(pair: Pair<Rule>) {
        // A pair is a combination of the rule which matched and a span of input
        println!("Rule:    {:?}", pair.as_rule());
        println!("Span:    {:?}", pair.as_span());
        println!("Text:    {}", pair.as_str());

        for inner_pair in pair.into_inner() {
            process_pair(inner_pair);
        }
    }

    fn print_parse_result(parse_result: Result<Pairs<Rule>, pest::error::Error<Rule>>) {
        let parse_result_clone = parse_result.clone();
        match parse_result {
            Ok(pairs) => {
                for pair in pairs {
                    // A pair is a combination of the rule which matched and a span of input
                    process_pair(pair);
                }
            }
            Err(err) => {
                // Handle parsing error
                println!("Error: {}", err);
            }
        }

        assert!(!parse_result_clone.is_err());
    }

    fn extract_range(input: &str) -> Option<(&str, i32, i32)> {
        if let Some(open_bracket) = input.find('[') {
            if let Some(close_bracket) = input.find(']') {
                if let Some(colon) = input.find(':') {
                    let name = &input[..open_bracket];
                    let start = input[open_bracket + 1..colon].parse().ok()?;
                    let end = input[colon + 1..close_bracket].parse().ok()?;
                    return Some((name, start, end));
                }
            }
        }
        None
    }
    
    fn extract_single(input: &str) -> Option<(&str, i32)> {
        if let Some(open_bracket) = input.find('[') {
            if let Some(close_bracket) = input.find(']') {
                let name = &input[..open_bracket];
                let index = input[open_bracket + 1..close_bracket].parse().ok()?;
                return Some((name, index));
            }
        }
        None
    }
    
    fn extract_name(input: &str) -> Option<&str> {
        Some(input)
    }

    #[test]
    fn test_parse_comment() {
        let input_str = "/*test
        test
        */";
        let parse_result = VerilogParser::parse(Rule::COMMENT, input_str);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_port_or_wire_id() {
        let input_str = "rid_nic400_axi4_ps2_1_";
        let parse_result = VerilogParser::parse(Rule::port_or_wire_id, input_str);

        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_port_list() {
        let input_str = "in1, in2, clk1, clk2, clk3, out";
        let parse_result = VerilogParser::parse(Rule::port_list, input_str);
        println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_port_or_wire_id1() {
        let input_str = "\\clk_cfg[6]";
        let parse_result = VerilogParser::parse(Rule::port_or_wire_id, input_str);

        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_input_declaration() {
        let input_str = "input chiplink_rx_clk_pad;";
        let parse_result = VerilogParser::parse(Rule::input_declaration, input_str);

        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_input_declaration1() {
        let input_str = "input [1:0] din;";
        let parse_result = VerilogParser::parse(Rule::input_declaration, input_str);
        println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_port_block_declaration() {
        let input_str = r#"output [3:0] osc_25m_out_pad;
        input osc_100m_in_pad;
        output osc_100m_out_pad;"#;
        let parse_result = VerilogParser::parse(Rule::port_block_declaration, input_str);
        println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_wire_declaration() {
        let _input_str = "wire \\vga_b[0] ;";
        let input_str1 = "wire ps2_dat;";
        let parse_result = VerilogParser::parse(Rule::wire_declaration, input_str1);

        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_wire_block_declaration() {
        let input_str = r#"wire \core_dip[2] ;
        wire \core_dip[1] ;
        wire \core_dip[0] ;
        wire \u0_rcg/u0_pll_bp ;
        wire \u0_rcg/u0_pll_fbdiv_5_ ;
        wire \u0_rcg/u0_pll_postdiv2_1_ ;
        wire \u0_rcg/u0_pll_clk ;"#;
        let parse_result = VerilogParser::parse(Rule::wire_block_declaration, input_str);

        print_parse_result(parse_result);
    }


    #[test] 
    fn test_parse_first_port_connection_single_connect() {
        let input_str = r#".I(\u0_soc_top/u0_ysyx_210539/writeback_io_excep_en )"#;
        let parse_result = VerilogParser::parse(Rule::first_port_connection_single_connect, input_str);

        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_first_port_connection_multiple_connect() {
        let input_str = r#".rid_nic400_axi4_ps2({ 1'b0,1'b0,rid_nic400_axi4_ps2_1_,1'b0 })"#;
        let parse_result = VerilogParser::parse(Rule::first_port_connection_multiple_connect, input_str);
        println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }


    #[test] 
    fn test_parse_port_connection() {
        let input_str = r#",
        .Z(hold_net_52144)"#;
        let parse_result = VerilogParser::parse(Rule::port_connection, input_str);

        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_port_block_connection() {
        let input_str = r#"(.BYPASS(\u0_rcg/u0_pll_bp ),
        .REFDIV({ DRV_net_6,
                DRV_net_6,
                DRV_net_6,
                DRV_net_6,
                DRV_net_7,
                DRV_net_6 }),
        .POSTDIV2({ DRV_net_6,
                FE_PDN5026_u0_rcg_u0_pll_postdiv2_1,
                \u0_rcg/n34  }),
        .DSMPD(DRV_net_7),
        .FOUTPOSTDIVPD(DRV_net_6),
        .POSTDIV1({ \u0_rcg/n37 ,
                FE_PDN3515_pll_cfg_2,
                FE_PDN4015_u0_rcg_n35 }),
        .PD(DRV_net_6),
        .FOUTVCOPD(DRV_net_6),
        .FBDIV({ FE_PDN11668_DRV_net_6,
                FE_PDN11668_DRV_net_6,
                FE_PDN11668_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                DRV_net_7,
                FE_PDN4133_u0_rcg_u0_pll_fbdiv_5,
                \u0_rcg/n36 ,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6 }),
        .FREF(FE_ECON20449_sys_clk_25m_buf),
        .FOUTVCO(),
        .CLKSSCG(),
        .LOCK(),
        .FOUTPOSTDIV(\u0_rcg/u0_pll_clk ));"#;

        let parse_result = VerilogParser::parse(Rule::port_block_connection, input_str);
        println!("{:#?}",parse_result);
        // print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_inst_declaration() {
        let input_str = r#"PLLTS28HPMLAINT \u0_rcg/u0_pll  (.BYPASS(\u0_rcg/u0_pll_bp ),
        .REFDIV({ DRV_net_6,
                DRV_net_6,
                DRV_net_6,
                DRV_net_6,
                DRV_net_7,
                DRV_net_6 }),
        .POSTDIV2({ DRV_net_6,
                FE_PDN5026_u0_rcg_u0_pll_postdiv2_1,
                \u0_rcg/n34  }),
        .DSMPD(DRV_net_7),
        .FOUTPOSTDIVPD(DRV_net_6),
        .POSTDIV1({ \u0_rcg/n37 ,
                FE_PDN3515_pll_cfg_2,
                FE_PDN4015_u0_rcg_n35 }),
        .PD(DRV_net_6),
        .FOUTVCOPD(DRV_net_6),
        .FBDIV({ FE_PDN11668_DRV_net_6,
                FE_PDN11668_DRV_net_6,
                FE_PDN11668_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                DRV_net_7,
                FE_PDN4133_u0_rcg_u0_pll_fbdiv_5,
                \u0_rcg/n36 ,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6,
                FE_PDN1270_DRV_net_6 }),
        .FREF(FE_ECON20449_sys_clk_25m_buf),
        .FOUTVCO(),
        .CLKSSCG(),
        .LOCK(),
        .FOUTPOSTDIV(\u0_rcg/u0_pll_clk ));"#;
        let parse_result = VerilogParser::parse(Rule::inst_declaration, input_str);
        println!("{:#?}",parse_result);
        // print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_module_id() {
        let input_str = "soc_top_0";
        let parse_result = VerilogParser::parse(Rule::module_id, input_str);
        println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_inst_block_declaration() {
        let input_str = r#"DEL150MD1BWP40P140HVT hold_buf_52163 (.I(\u0_soc_top/u0_ysyx_210539/csrs/n3692 ),
        .Z(hold_net_52163));
        DEL150MD1BWP40P140HVT hold_buf_52164 (.I(\u0_soc_top/u0_ysyx_210539/icache/Ram_bw_3_io_wdata[123] ),
        .Z(hold_net_52164));"#;
        let parse_result = VerilogParser::parse(Rule::inst_block_declaration, input_str);
        println!("{:#?}",parse_result);
        // print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_module_declaration() {
        let input_str = r#"module preg_w4_reset_val0_0 (
            clock,
            reset,
            din,
            dout,
            wen);
        input clock;
        input reset;
        input [3:0] din;
        output [3:0] dout;
        input wen;
        
        // Internal wires
        wire n4;
        wire n1;
        
        DFQD1BWP40P140 data_reg_1_ (.CP(clock),
            .D(n4),
            .Q(dout[1]));
        MUX2NUD1BWP40P140 U3 (.I0(dout[1]),
            .I1(din[1]),
            .S(wen),
            .ZN(n1));
        NR2D1BWP40P140 U4 (.A1(reset),
            .A2(n1),
            .ZN(n4));
        endmodule"#;
        let parse_result = VerilogParser::parse(Rule::module_declaration, input_str);
        println!("{:#?}",parse_result);
        // print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_verilog_file1() {
        let verilog_file_path  = "/home/longshuaiying/iEDA/src/database/manager/parser/verilog/verilog-rust/verilog-parser/example/asic_top_flatten.v";
        let input_str =
        std::fs::read_to_string(verilog_file_path).unwrap_or_else(|_| panic!("Can't read file: {}", verilog_file_path));
       
        let parse_result = VerilogParser::parse(Rule::verilog_file, input_str.as_str());
        // println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_verilog_file2() {
        let verilog_file_path  = "/home/longshuaiying/iEDA/src/database/manager/parser/verilog/verilog-rust/verilog-parser/example/example1.v";
        let input_str =
        std::fs::read_to_string(verilog_file_path).unwrap_or_else(|_| panic!("Can't read file: {}", verilog_file_path));
       
        let parse_result = VerilogParser::parse(Rule::verilog_file, input_str.as_str());
        // println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_wire_list_with_scalar_constant() {
        let _input_str  = r#"1'b0, 1'b0, rid_nic400_axi4_ps2_1_, 1'b0"#;
        let input_str1 = r#"\u0_rcg/n33 ,  \u0_rcg/n33 ,  \u0_rcg/n33"#;

        let parse_result = VerilogParser::parse(Rule::wire_list_with_scalar_constant, input_str1);
        println!("{:#?}",parse_result);
        print_parse_result(parse_result);
    }

    #[test] 
    fn test_parse_verilog_file3() {
        let verilog_file_path  = "/home/longshuaiying/iEDA/src/database/manager/parser/verilog/verilog-rust/verilog-parser/example/asic_top_DC_downsize.v";
        let input_str =
        std::fs::read_to_string(verilog_file_path).unwrap_or_else(|_| panic!("Can't read file: {}", verilog_file_path));
       
        let parse_result = VerilogParser::parse(Rule::verilog_file, input_str.as_str());
        println!("{:#?}",parse_result);
        // print_parse_result(parse_result);
    }
    
    #[test] 
    fn test_extract_funs() {
        let _input1 = "gpio[3:0]";
        let _input2 = "gpio[0]";
        let input3 = "gpio";
        if let Some((name, range_from, range_to)) = extract_range(input3) {
            // extract gpio，3，0
            println!("extract_range:name={}, range_from={}, range_to={}", name, range_from, range_to);
        } else if let Some((name, index)) = extract_single(input3) {
            // extract:gpio，0
            println!("extract_single:name={}, index={}", name, index);
        } else if let Some(name) = extract_name(input3) {
            // extract:gpio
            println!("extract_name:name={}", name);
        } else {
            panic!("error format!");
        }
    }

}