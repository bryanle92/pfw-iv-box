#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <linux/fb.h>
#include <linux/uaccess.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/device.h>

#define SSD1309_WIDTH      128
#define SSD1309_HEIGHT     64
#define SSD1309_BUF_SIZE   (SSD1309_WIDTH * SSD1309_HEIGHT / 8)

struct oled_device {
    struct i2c_client *client;
    struct regmap *regmap;
    struct fb_info *info;
    u8 *buffer;
    struct mutex lock;
};

static const struct regmap_config ssd1309_regmap_config = {
    .reg_bits = 8,
    .val_bits = 8,
    .cache_type = REGCACHE_NONE,
};

static void ssd1309fb_update_display(struct oled_device *oled)
{
    int page, i;
    
    for (page = 0; page < 8; page++) {
        int offset = page * 128;
        
        /* Set page address */
        regmap_write(oled->regmap, 0x00, 0xB0 + page);
        
        /* Set column address range: 0 to 127 */
        regmap_write(oled->regmap, 0x00, 0x21);
        regmap_write(oled->regmap, 0x00, 0x00);
        regmap_write(oled->regmap, 0x00, 0x7F);
        
        /* Write 128 bytes of display data */
        for (i = 0; i < 128; i++) {
            regmap_write(oled->regmap, 0x40, oled->buffer[offset + i]);
        }
    }
}

static void ssd1309_hw_init(struct oled_device *oled)
{
    /* Turn display OFF */
    regmap_write(oled->regmap, 0x00, 0xAE);
    
    /* Set segment remap */
    regmap_write(oled->regmap, 0x00, 0xA1);
    
    /* Set COM scan direction */
    regmap_write(oled->regmap, 0x00, 0xC8);
    
    /* Set display offset */
    regmap_write(oled->regmap, 0x00, 0xD3);
    regmap_write(oled->regmap, 0x00, 0x00);
    
    /* Set display start line */
    regmap_write(oled->regmap, 0x00, 0x40);
    
    /* Set multiplex ratio */
    regmap_write(oled->regmap, 0x00, 0xA8);
    regmap_write(oled->regmap, 0x00, 0x3F);
    
    /* Set COM pins configuration */
    regmap_write(oled->regmap, 0x00, 0xDA);
    regmap_write(oled->regmap, 0x00, 0x12);
    
    /* Set normal display mode */
    regmap_write(oled->regmap, 0x00, 0xA6);
    
    /* Set column address range */
    regmap_write(oled->regmap, 0x00, 0x21);
    regmap_write(oled->regmap, 0x00, 0x00);
    regmap_write(oled->regmap, 0x00, 0x7F);
    
    /* Set page address range */
    regmap_write(oled->regmap, 0x00, 0x22);
    regmap_write(oled->regmap, 0x00, 0x00);
    regmap_write(oled->regmap, 0x00, 0x07);
    
    /* Turn display ON */
    regmap_write(oled->regmap, 0x00, 0xAF);
}

static ssize_t ssd1309fb_write(struct fb_info *info, const char __user *buf,
                               size_t count, loff_t *ppos)
{
    struct oled_device *oled = info->par;
    size_t to_copy = min_t(size_t, count, SSD1309_BUF_SIZE);
    
    mutex_lock(&oled->lock);
    
    if (copy_from_user(oled->buffer, buf, to_copy)) {
        mutex_unlock(&oled->lock);
        return -EFAULT;
    }
    
    memcpy(info->screen_base, oled->buffer, to_copy);
    ssd1309fb_update_display(oled);
    
    mutex_unlock(&oled->lock);
    return to_copy;
}


static struct fb_ops ssd1309fb_ops = {
    .owner = THIS_MODULE,
    .fb_write = ssd1309fb_write,
    .fb_fillrect = sys_fillrect,
    .fb_copyarea = sys_copyarea,
    .fb_imageblit = sys_imageblit,
};

static ssize_t display_on_show(struct device *dev,
                                struct device_attribute *attr, char *buf)
{
    return sysfs_emit(buf, "1\n");
}

static ssize_t display_on_store(struct device *dev,
                                 struct device_attribute *attr,
                                 const char *buf, size_t count)
{
    struct fb_info *info = dev_get_drvdata(dev);
    struct oled_device *oled = info->par;
    bool on;
    
    if (kstrtobool(buf, &on) < 0)
        return -EINVAL;
    
    mutex_lock(&oled->lock);
    regmap_write(oled->regmap, 0x00, on ? 0xAF : 0xAE);
    mutex_unlock(&oled->lock);
    
    return count;
}

static DEVICE_ATTR_RW(display_on);

static int ssd1309fb_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct fb_info *info;
    struct oled_device *oled;
    u8 *fb_buffer;
    int ret;
    
    info = framebuffer_alloc(sizeof(struct oled_device), &client->dev);
    if (!info)
        return -ENOMEM;
    
    oled = info->par;
    oled->client = client;
    mutex_init(&oled->lock);
    
    oled->regmap = devm_regmap_init_i2c(client, &ssd1309_regmap_config);
    if (IS_ERR(oled->regmap)) {
        ret = PTR_ERR(oled->regmap);
        goto err_fb_release;
    }
    
    oled->buffer = devm_kzalloc(&client->dev, SSD1309_BUF_SIZE, GFP_KERNEL);
    if (!oled->buffer) {
        ret = -ENOMEM;
        goto err_fb_release;
    }
    
    fb_buffer = vzalloc(SSD1309_BUF_SIZE);
    if (!fb_buffer) {
        ret = -ENOMEM;
        goto err_fb_release;
    }
    
    oled->info = info;
    info->screen_base = (char __force __iomem *)fb_buffer;
    info->fbops = &ssd1309fb_ops;
    info->fix = (struct fb_fix_screeninfo) {
        .id = "ssd1309fb",
        .type = FB_TYPE_PACKED_PIXELS,
        .visual = FB_VISUAL_MONO10,
        .line_length = SSD1309_WIDTH / 8,
        .smem_len = SSD1309_BUF_SIZE,
    };
    info->var = (struct fb_var_screeninfo) {
        .xres = SSD1309_WIDTH,
        .yres = SSD1309_HEIGHT,
        .xres_virtual = SSD1309_WIDTH,
        .yres_virtual = SSD1309_HEIGHT,
        .bits_per_pixel = 1,
    };
    info->par = oled;
    
    ssd1309_hw_init(oled);
    
    ret = register_framebuffer(info);
    if (ret < 0)
        goto err_vfree;
    
    i2c_set_clientdata(client, info);
    dev_set_drvdata(&client->dev, info);
    
    ret = device_create_file(&client->dev, &dev_attr_display_on);
    if (ret < 0)
        goto err_unregister_fb;
    
    dev_info(&client->dev, "SSD1309 framebuffer registered at /dev/fb0\n");
    return 0;
    
err_unregister_fb:
    unregister_framebuffer(info);
err_vfree:
    vfree(fb_buffer);
err_fb_release:
    framebuffer_release(info);
    return ret;
}

static void ssd1309fb_remove(struct i2c_client *client)
{
    struct fb_info *info = i2c_get_clientdata(client);
    
    device_remove_file(&client->dev, &dev_attr_display_on);
    unregister_framebuffer(info);
    vfree(info->screen_base);
    framebuffer_release(info);
    
    dev_info(&client->dev, "SSD1309 framebuffer removed\n");
}

static const struct i2c_device_id ssd1309fb_id[] = {
    { "ssd1309fb", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ssd1309fb_id);

static const struct of_device_id ssd1309fb_dt_ids[] = {
    { .compatible = "solomon,ssd1309" },
    { }
};
MODULE_DEVICE_TABLE(of, ssd1309fb_dt_ids);

static struct i2c_driver ssd1309fb_driver = {
    .driver = {
        .name = "ssd1309fb",
        .of_match_table = ssd1309fb_dt_ids,
    },
    .probe = ssd1309fb_probe,
    .remove = ssd1309fb_remove,
    .id_table = ssd1309fb_id,
};

module_i2c_driver(ssd1309fb_driver);

MODULE_AUTHOR("Platform Interview");
MODULE_DESCRIPTION("SSD1309 OLED framebuffer driver - Complete the implementation");
MODULE_LICENSE("GPL");
