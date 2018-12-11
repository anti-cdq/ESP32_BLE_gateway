/* SD card and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <string.h>
#include <sys/dirent.h>
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

static const char *TAG = "SD CARD";


#define FILE_PATH_LEN			50
#define FILE_NUM				50
#define FILE_NAME_LEN			30
#define FILE_BUFF_LEN			1024
#define LCD_BUFF_LEN			(LCD_W*2)

char *all_file_name;
char *file_path;
uint8_t *Buff;
uint8_t *buff_to_lcd;
static uint8_t display_flag = 0;

void show_bmp_center(char* path)
{
	uint16_t x1=0, y1=0, x2=LCD_W, y2=LCD_H;		//图片显示区域
	uint16_t offset_width = 0, offset_height = 0;
	uint16_t image_width = 0, image_height = 0;
	uint16_t display_width = 0;
	uint16_t x = 0, y = 0;
	uint32_t temp = 0;

	memset(buff_to_lcd, 0, LCD_BUFF_LEN);
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
		display_width = image_width;
	}
	else
	{
		display_width = LCD_W;
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
		fread((char *)Buff, display_width*3, 1, f);
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
		printf("d_ino: %d ", ptr->d_ino);
		printf("d_type: %d ", ptr->d_type);
		printf("dd_vfs_idx: %d ", dir->dd_vfs_idx);
		printf("dd_rsv: %d ", dir->dd_rsv);

		sprintf(file_path, "%.*s%.*s%.*s", strlen(path),path,strlen("/"), "/", strlen(ptr->d_name), ptr->d_name);
		show_bmp_info(file_path);
		display_flag = 1;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	closedir(dir);
}


void sd_card_task_mem_free(void)
{
    // All done, unmount partition and disable SDMMC or SPI peripheral
    esp_vfs_fat_sdmmc_unmount();
    ESP_LOGI(TAG, "Card unmounted");

	free(all_file_name);
	free(file_path);
	free(Buff);
	free(buff_to_lcd);
}


void sd_card_info_display(void)
{
	if(display_flag)
	{
		show_bmp_center(file_path);
		display_flag = 0;
	}
}


void sd_card_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    all_file_name = (char *)malloc(FILE_NUM*FILE_NAME_LEN);
    file_path = (char *)malloc(FILE_PATH_LEN);
    Buff = (uint8_t *)malloc(FILE_BUFF_LEN);
    buff_to_lcd = (uint8_t *)malloc(LCD_BUFF_LEN);

    gpio_set_pull_mode(15, GPIO_PULLUP_ONLY);   // CMD, needed in 4- and 1- line modes
    gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);    // D0, needed in 4- and 1-line modes
    gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);    // D1, needed in 4-line mode only
    gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);   // D2, needed in 4-line mode only
    gpio_set_pull_mode(13, GPIO_PULLUP_ONLY);   // D3, needed in 4- and 1-line modes

    esp_vfs_fat_sdmmc_mount_config_t mount_config =
    {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };

    sdmmc_card_t* card;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                "If you want the card to be formatted, set format_if_mount_failed = true.");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }

    // Card has been initialized, print its properties
    sdmmc_card_print_info(stdout, card);
    scan_files("/sdcard/all");

    while(1)
    {
    	vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

