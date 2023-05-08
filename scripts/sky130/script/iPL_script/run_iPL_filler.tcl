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
##   read def
#===========================================================
def_init -path ./result/iRT_result.def

#===========================================================
##   run Filler
#===========================================================
run_filler -config ./iEDA_config/pl_default_config.json

#===========================================================
##   Save def 
#===========================================================
def_save -path ./result/iPL_filler_result.def

#===========================================================
##   Save netlist 
#===========================================================
netlist_save -path ./result/iPL_filler_result.v -exclude_cell_names {}

#===========================================================
##   report 
#===========================================================
report_db -path "./result/report/filler_db.rpt"

#===========================================================
##   Exit 
#===========================================================
flow_exit
