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
def_init -path ./result/iPL_filler_result.def

#===========================================================
##   save json 
##   Full layer information instance:(-discard li/mcon/nwell/pwell/met/via)
##   use (-discard null) to choose all layer
#===========================================================
json_save -path ./result/final_design.json -discard li

#===========================================================
##   Exit 
#=======================                                 ====================================
flow_exit