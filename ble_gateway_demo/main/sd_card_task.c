/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "lcd.h"
#include "global_config.h"

static const char *TAG = "example";

// This example can use SDMMC and SPI peripherals to communicate with SD card.
// By default, SDMMC peripheral is used.
// To enable SPI mode, uncomment the following line:

// #define USE_SPI_MODE

// When testing SD and SPI modes, keep in mind that once the card has been
// initialized in SPI mode, it can not be reinitialized in SD mode without
// toggling power to the card.

#ifdef USE_SPI_MODE
// Pin mapping when using SPI mode.
// With this mapping, SD card can be used both in SPI and 1-line SD mode.
// Note that a pull-up on CS line is required in SD mode.
	#define PIN_NUM_MISO 2
	#define PIN_NUM_MOSI 15
	#define PIN_NUM_CLK  14
	#define PIN_NUM_CS   13
#endif //USE_SPI_MODE


char file_path[50];
uint8_t Buff[1024];
uint8_t buff_to_lcd[LCD_W*2];


void show_bmp_center(char* path)
{
	uint16_t x1=0, y1=0, x2=LCD_W, y2=LCD_H;		//图片显示区域
	uint16_t offset_width = 0, offset_height = 0;
	uint16_t image_width = 0, image_height = 0;
	uint16_t x = 0, y = 0;
	uint32_t temp = 0;

	memset(buff_to_lcd, 0, LCD_W*2);
	LCD_Clear(BLACK);

	FILE *f = fopen(path, "r");
	if (f == NULL)
	{
		ESP_LOGE(TAG, "Failed to open file for reading");
		return;
	}
	fgets((char *)Buff, 54, f);

	image_width = (uint16_t)Buff[19]<<8 | (uint16_t)Buff[18];	//获取图片宽度信息
	image_height = (uint16_t)Buff[23]<<8 | (uint16_t)Buff[22];	//获取图片高度信息

	if(image_width < LCD_W)				//要显示的图片宽度小于LCD宽度，则居中显示
	{
		x1 = (LCD_W - image_width)/2;	//显示开始的横坐标
		x2 = x1 + image_width;			//显示结束的横坐标
	}
	else
	{
		offset_width = (image_width - LCD_W)/2;
	}

	if(image_height < LCD_H)
	{
		y1 = (LCD_H - image_height)/2;
		y2 = y1 + image_height;
	}
	else
	{
		offset_height = (image_height - LCD_H)/2;
	}

	temp = 54 + (image_width*3+image_width%4)*(image_height-1) - image_width*3*offset_height + 3*offset_width;	//计算将要显示的第一行第一个像素的位置
	fseek (f, temp, SEEK_SET);

	Address_set(x1, y1, x2-1, y2-1);	//设置快速填充窗口
	for(y=y1;y<y2;y++)
	{
		fgets((char *)Buff, image_width*3, f);
		for(x=0;x<image_width;x++)
		{
			buff_to_lcd[x*2] = (Buff[x*3+2] & 0xF8) | (Buff[x*3+1]>>5);
			buff_to_lcd[x*2+1] = ((Buff[x*3+1] & 0x1C)<<3) | (Buff[x*3]>>3);
		}
		lcd_write_data(buff_to_lcd, image_width*2);

		temp = temp - 3*image_width - image_width%4;
		fseek (f, temp, SEEK_SET);
	}

	fclose(f);
}


void show_bmp_info(char* path)
{
	uint16_t image_width = 0, image_height = 0;
	uint8_t line[64];

	FILE *f = fopen(path, "r");
	if (f == NULL) {
		ESP_LOGE(TAG, "Failed to open file for reading");
		return;
	}
	fgets((char *)line, sizeof(line), f);
	fclose(f);

	image_width = (uint16_t)line[19]<<8 | (uint16_t)line[18];	//获取图片宽度信息
	image_height = (uint16_t)line[23]<<8 | (uint16_t)line[22];	//获取图片高度信息

	printf("size is: %d*%d\n", image_width, image_height);
}


void scan_files(char* path)
{
	DIR * dir;
	struct dirent * ptr;
	dir = opendir(path);

	memset(file_path, 0, 50);
	while((ptr = readdir(dir)) != NULL)
	{
		printf("d_name : %s ", ptr->d_name);
		sprintf(file_path, "%.*s%.*s%.*s", strlen(path),path,strlen("/"), "/", strlen(ptr->d_name), ptr->d_name);
		show_bmp_info(file_path);
		xEventGroupSetBits(ble_event_group, LCD_DISPLAY_UPDATE_BIT);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	closedir(dir);
}


void sd_card_task_mem_free(void)
{

//	free(scan_ap_num);
//	free(scan_flag);
//	free(scan_result);
//	free(print_temp);
//	free(wifi_scan_config);
}


void sd_card_info_display(void)
{
	show_bmp_center(file_path);
}


void sd_card_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Initializing SD card");

#ifndef USE_SPI_MODE
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();

    // To use 1-line SD mode, uncomment the following line:
    // host.flags = SDMMC_HOST_FLAG_1BIT;

    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
    // Internal pull-ups are not sufficient. However, enabling internal pull-ups
    // does make a difference some boards, so we do that here.
    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

#else
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
    slot_config.gpio_miso = PIN_NUM_MISO;
    slot_config.gpio_mosi = PIN_NUM_MOSI;
    slot_config.gpio_sck  = PIN_NUM_CLK;
    slot_config.gpio_cs   = PIN_NUM_CS;
    // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
#endif //USE_SPI_MODE

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc_mount is an all-in-one convenience function.
    // Please check its source code and implement error recovery when developing
    // production applications.
    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);

    scan_files("/sdcard/beauty");

    // Use POSIX and C standard library functions to work with files.
    // First create a file.
    ESP_LOGI(TAG, "Opening file");
    FILE* f = fopen("/sdcard/hello.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        return;
    }
    fprintf(f, "Hello %s!\n", card->cid.name);
    fclose(f);
    ESP_LOGI(TAG, "File written");

    // Check if destination file exists before renaming
    struct stat st;
    if (stat("/sdcard/foo.txt", &st) == 0) {
        // Delete it if it exists
        unlink("/sdcard/foo.txt");
    }

    // Rename original file
    ESP_LOGI(TAG, "Renaming file");
    if (rename("/sdcard/hello.txt", "/sdcard/foo.txt") != 0) {
        ESP_LOGE(TAG, "Rename failed");
        return;
    }

    // Open renamed file for reading
    ESP_LOGI(TAG, "Reading file");
    f = fopen("/sdcard/foo.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        return;
    }
    char line[64];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char* pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
    ESP_LOGI(TAG, "Read from file: '%s'", line);

    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(TAG, "Card unmounted");

    while(1)
    {
    	vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

