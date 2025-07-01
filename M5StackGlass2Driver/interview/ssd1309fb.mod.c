#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xb1ad28e0, "__gnu_mcount_nc" },
	{ 0x36948ecd, "i2c_register_driver" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
	{ 0xebe3c962, "device_remove_file" },
	{ 0xa33837bb, "unregister_framebuffer" },
	{ 0x999e8297, "vfree" },
	{ 0xf45c5cba, "framebuffer_release" },
	{ 0x154aa398, "_dev_info" },
	{ 0x124bad4d, "kstrtobool" },
	{ 0x828ce6bb, "mutex_lock" },
	{ 0xb3b58d73, "regmap_write" },
	{ 0x9618ede0, "mutex_unlock" },
	{ 0x3ea1b6e4, "__stack_chk_fail" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0xe783e261, "sysfs_emit" },
	{ 0xae353d77, "arm_copy_from_user" },
	{ 0x9d669763, "memcpy" },
	{ 0x5f754e5a, "memset" },
	{ 0xadc90aae, "framebuffer_alloc" },
	{ 0xde4bf88b, "__mutex_init" },
	{ 0x7920dca, "__devm_regmap_init_i2c" },
	{ 0xc4bda6b9, "devm_kmalloc" },
	{ 0x40a9b349, "vzalloc" },
	{ 0xac193cc5, "register_framebuffer" },
	{ 0xe6df3144, "device_create_file" },
	{ 0xec85562e, "i2c_del_driver" },
	{ 0x441afa24, "sys_fillrect" },
	{ 0x51134678, "sys_copyarea" },
	{ 0xdd9dff02, "sys_imageblit" },
	{ 0xc84d16dc, "module_layout" },
};

MODULE_INFO(depends, "regmap-i2c,sysfillrect,syscopyarea,sysimgblt");

MODULE_ALIAS("of:N*T*Csolomon,ssd1309");
MODULE_ALIAS("of:N*T*Csolomon,ssd1309C*");
MODULE_ALIAS("i2c:ssd1309fb");

MODULE_INFO(srcversion, "057D2E3AFCC3957470F48C7");
