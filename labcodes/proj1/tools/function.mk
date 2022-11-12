OBJPREFIX	:= __objs_

.SECONDEXPANSION:
#####################function begin###########################

# 列举文件夹$(1)中满足条件$(2)的所有文件。
# addprefix给$(2)所有文件加上前缀$(2).？
# if函数判断$(2)是否非空，如果非空执行第一句这个%不是模式的意思，就是单纯的字符串%.
# 即给$(2)中每个单词加上前缀%.变为%.suf
# ，否则执行第二句
# addsuffix函数给$(1)都加上后缀/*
# wildcard返回$(1)/*下所有的文件(root/name.suf)
# 最后在第二个结果中过滤满足第一个结果的所有单词
listf = $(filter $(if $(2),$(addprefix %.,$(2)),%),\
				$(wildcard $(addsuffix $(SLASH)*,$(1))))

# 第一个参数为
# 把文件名去掉后缀后添加.o后缀，并添加obj/前缀
# basename提取$(1)的没有后缀的文件名
# addsuffix为前一个结果添加后缀.o
# 添加obj/前缀
toobj = $(addprefix $(OBJDIR)$(SLASH)$(if $(2),$(2)$(SLASH)),\
		$(addsuffix .o,$(basename $(1))))

# 替换obj.o为obj.d
todep = $(patsubst %.o,%.d,$(call toobj,$(1),$(2)))

# 为每个文件添加bin/前缀
totarget = $(addprefix $(BINDIR)$(SLASH),$(1))

# 为每个文件添加__objs_前缀
packetname = $(if $(1),$(addprefix $(OBJPREFIX),$(1)),$(OBJPREFIX))

# $1=source file
# $2=gcc
# $3=cflags
# $4=empty
# | $$$$(dir $$$$@)是干嘛的？
# 生成多目标及其依赖
define cc_template
$$(call todep,$(1),$(4)): $(1) | $$$$(dir $$$$@)
	@$(2) -I$$(dir $(1)) $(3) -MM $$< -MT "$$(patsubst %.d,%.o,$$@) $$@"> $$@
$$(call toobj,$(1),$(4)): $(1) | $$$$(dir $$$$@)
	@echo + cc $$<
	$(V)$(2) -I$$(dir $(1)) $(3) -c $$< -o $$@
ALLOBJS += $$(call toobj,$(1),$(4))
endef

define do_cc_compile
$$(foreach f,$(1),$$(eval $$(call cc_template,$$(f),$(2),$(3),$(4))))
endef

# $1=所有文件
# $2=编译器
# $3=编译选项
# $4=包名
# $5=
# 对$4每个文件添加前缀成为package
define do_add_files_to_packet
__temp_packet__ := $(call packetname,$(4))
ifeq ($$(origin $$(__temp_packet__)),undefined)
$$(__temp_packet__) :=
endif
__temp_objs__ := $(call toobj,$(1),$(5))
$$(foreach f,$(1),$$(eval $$(call cc_template,$$(f),$(2),$(3),$(5))))
$$(__temp_packet__) += $$(__temp_objs__)
endef

define do_add_objs_to_packet
__temp_packet__ := $(call packetname,$(2))
ifeq ($$(origin $$(__temp_packet__)),undefined)
$$(__temp_packet__) :=
endif
$$(__temp_packet__) += $(1)
endef

define do_create_target
__temp_target__ = $(call totarget,$(1))
__temp_objs__ = $$(foreach p,$(call packetname,$(2)),$$($$(p))) $(3)
TARGETS += $$(__temp_target__)
ifneq ($(4),)
$$(__temp_target__): $$(__temp_objs__) | $$$$(dir $$$$@)
	$(V)$(4) $(5) $$^ -o $$@
else
$$(__temp_target__): $$(__temp_objs__) | $$$$(dir $$$$@)
endif
endef

define do_finish_all
ALLDEPS = $$(ALLOBJS:.o=.d)
$$(sort $$(dir $$(ALLOBJS)) $(BINDIR)$(SLASH) $(OBJDIR)$(SLASH)):
	@echo + mkdir $$@
	@$(MKDIR) $$@
endef

###########################function end#######################################

# 编译文件，$1=files, $2=cc, $3=cflag, $4=dir
cc_compile = $(eval $(call do_cc_compile,$(1),$(2),$(3),$(4)))

# 添加文件
add_files = $(eval $(call do_add_files_to_packet,$(1),$(2),$(3),$(4),$(5)))

# 添加中间文件到包
add_objs = $(eval $(call do_add_objs_to_packet,$(1),$(2)))

read_packet = $(foreach p,$(call packetname,$(1)),$($(p)))

# 添加包和中间文件到目标
create_target = $(eval $(call do_create_target,$(1),$(2),$(3),$(4),$(5)))

finish_all = $(eval $(call do_finish_all))
