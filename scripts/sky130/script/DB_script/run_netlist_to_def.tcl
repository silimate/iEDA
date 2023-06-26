#===========================================================
##   init flow config
#===========================================================
flow_init -config ./iEDA_config/flow_config.json

#===========================================================
##   read db config
#===========================================================
db_init -config ./iEDA_config/db_default_config.json

#===========================================================
##   reset data path
#===========================================================
source ./script/DB_script/db_path_setting.tcl

#===========================================================
##   read lef
#===========================================================
source ./script/DB_script/db_init_lef.tcl

#===========================================================
##   read verilog
#===========================================================
verilog_init -path ./result/verilog/gcd.v -top gcd

#===========================================================
##   save def 
#===========================================================
def_save -path ./result/netlist_result.def

#===========================================================
##   save verilog 
#===========================================================
netlist_save -path ./result/netlist_result.v -exclude_cell_names {}

#===========================================================
##   Exit 
#===========================================================
flow_exit
