﻿# CMakeList.txt: Valgrind 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.9)

# 将源代码添加到此项目的可执行文件。
# add_compile_options(-std=c++17)
aux_source_directory(. DIR_Valgrind_SRCS)
project(Valgrind CXX)

INCLUDE_DIRECTORIES("priv/")
INCLUDE_DIRECTORIES("pub/")


add_library(Valgrind    "priv/main_main.c"
                        "priv/main_globals.c"
                        "priv/main_util.c"
                        "priv/multiarch_main_main.c"
                        )

add_library(ir_match "priv/ir_match.c"
                     "priv/ir_opt.c"
                     "priv/ir_inject.c"
                     "priv/ir_defs.c"
                     )


add_library(host_generic_maddf "priv/host_generic_maddf.c")
add_library(host_generic_simd   "priv/host_generic_simd64.c"
                                "priv/host_generic_simd128.c"
                                "priv/host_generic_simd256.c"
                                )
add_library(host_generic_regs   "priv/host_generic_regs.c"
                                "priv/host_generic_reg_alloc2.c"
                                "priv/host_generic_reg_alloc3.c"
                                )
 

add_library(guest_generic_bb_to_IR  "priv/guest_generic_bb_to_IR.c")	 
add_library(guest_generic_x87       "priv/guest_generic_x87.c")



add_library(host_x86_defs   "priv/host_x86_defs.c"
                            "priv/guest_x86_helpers.c"
                            "priv/host_x86_isel.c"
                            "priv/guest_x86_toIR.c"
                            )
add_library(host_amd64_defs "priv/host_amd64_defs.c"
                            "priv/guest_amd64_helpers.c"
                            "priv/host_amd64_isel.c"
                            "priv/guest_amd64_toIR.c"
                            )
add_library(host_arm_defs   "priv/host_arm_defs.c"
                            "priv/guest_arm_helpers.c"
                            "priv/host_arm_isel.c"
                            "priv/guest_arm_toIR.c"
                            )
add_library(host_arm64_defs "priv/host_arm64_defs.c"
                            "priv/guest_arm64_helpers.c"
                            "priv/host_arm64_isel.c"
                            "priv/guest_arm64_toIR.c"
                            )
add_library(host_ppc_defs   "priv/host_ppc_defs.c"
                            "priv/guest_ppc_helpers.c"
                            "priv/host_ppc_isel.c"
                            "priv/guest_ppc_toIR.c"
                            )
add_library(host_mips_defs  "priv/host_mips_defs.c"
                            "priv/guest_mips_helpers.c"
                            "priv/host_mips_isel.c"
                            "priv/guest_mips_toIR.c"
                            "priv/guest_mipsdsp_toIR.c"
                            )	   
add_library(host_s390_defs  "priv/s390_disasm.c"
                            "priv/host_s390_defs.c"
                            "priv/guest_s390_helpers.c"
                            "priv/host_s390_isel.c"
                            "priv/guest_s390_toIR.c"
                            )



TARGET_LINK_LIBRARIES(  Valgrind ir_match host_generic_maddf host_generic_simd host_generic_regs guest_generic_bb_to_IR guest_generic_x87
                        host_x86_defs host_amd64_defs host_arm_defs host_arm64_defs host_ppc_defs host_mips_defs host_s390_defs
                        )


# TODO: 如有需要，请添加测试并安装目标。