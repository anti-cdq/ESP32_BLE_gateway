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
#include "button.h"

static const char *TAG = "SD CARD";


#define FILE_PATH_LEN			50
#define FILE_NUM_PER_PAGE		10
#define FILE_NAME_LEN			30
#define FILE_BUFF_LEN			1024
#define LCD_BUFF_LEN			(LCD_W*2)

typedef struct
{
	uint8_t display_flag;
	uint8_t status;
	uint32_t file_num;
	uint32_t file_index;
	uint8_t	Buff[LCD_W*3];
	uint8_t buff_to_lcd[LCD_BUFF_LEN];
	char file_path[FILE_PATH_LEN];
	char all_file_name[FILE_NUM_PER_PAGE][FILE_NAME_LEN];
}file_browser_s;

file_browser_s *file_browser;
uint8_t *button_evt;


void show_bmp_center(char* path, uint16_t offsetX, uint16_t offsetY)
{
	uint16_t x1=0, y1=0, x2=LCD_W, y2=LCD_H;		//图片显示区域
	uint16_t offset_width = 0, offset_height = 0;
	uint16_t image_width = 0, image_height = 0;
	uint16_t display_width = 0;
	uint16_t x = 0, y = 0;
	uint32_t temp = 0;

	memset(file_browser->buff_to_lcd, 0, LCD_BUFF_LEN);
	LCD_Clear(BLACK);

	FILE *f = fopen(path, "r");
	if (f == NULL)
	{
		ESP_LOGE(TAG, "1.Failed to open file for reading");
		printf("%s\n", path);
		return;
	}
	fread((char *)file_browser->Buff, 54, 1, f);

	image_width = (uint16_t)file_browser->Buff[19]<<8 | (uint16_t)file_browser->Buff[18];	//获取图片宽度信息
	image_height = (uint16_t)file_browser->Buff[23]<<8 | (uint16_t)file_browser->Buff[22];	//获取图片高度信息

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

	temp = 54 + (image_width*3+image_width%4)*(image_height-offset_height-1) + 3*offset_width;	//计算将要显示的第一行第一个像素的位置
	fseek (f, temp, SEEK_SET);

	Address_set(x1, y1, x2-1, y2-1);	//设置快速填充窗口
	for(y=y1;y<y2;y++)
	{
		fread((char *)file_browser->Buff, display_width*3, 1, f);
		for(x=0;x<display_width;x++)
		{
			file_browser->buff_to_lcd[x*2] = (file_browser->Buff[x*3+2] & 0xF8) | (file_browser->Buff[x*3+1]>>5);
			file_browser->buff_to_lcd[x*2+1] = ((file_browser->Buff[x*3+1] & 0x1C)<<3) | (file_browser->Buff[x*3]>>3);
		}
		lcd_write_data(file_browser->buff_to_lcd, display_width*2);

		temp = temp - 3*image_width - image_width%4;
		fseek (f, temp, SEEK_SET);
	}

	fclose(f);
}


void show_bmp_info(char* path)
{
	uint16_t image_width = 0, image_height = 0;
	uint8_t line[54];

	FILE *f = fopen(path, "r");
	if (f == NULL) {
		ESP_LOGE(TAG, "2.Failed to open file for reading");
		return;
	}
	fread((char *)line, 54, 1, f);
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

	memset(file_browser->file_path, 0, FILE_PATH_LEN);
	while((ptr = readdir(dir)) != NULL)
	{
		printf("d_name : %s ", ptr->d_name);
		printf("d_ino: %d ", ptr->d_ino);
		printf("d_type: %d ", ptr->d_type);
		printf("dd_vfs_idx: %d ", dir->dd_vfs_idx);
		printf("dd_rsv: %d ", dir->dd_rsv);

		sprintf(file_browser->file_path,
				"%.*s%.*s%.*s",
				strlen(path),
				path,strlen("/"), "/",
				strlen(ptr->d_name),
				ptr->d_name);
		show_bmp_info(file_browser->file_path);
		file_browser->display_flag = 1;
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
	closedir(dir);
}


void sd_card_task_mem_free(void)
{
	// All done, unmount partition and disable SDMMC or SPI peripheral
	esp_vfs_fat_sdmmc_unmount();
	ESP_LOGI(TAG, "Card unmounted");

	free(file_browser);
	free(button_evt);
}


void sd_card_info_display(void)
{
	if(file_browser->display_flag == 1)
	{
		show_bmp_center(file_browser->file_path, 0, 0);
		file_browser->display_flag = 0;
	}
}


void sd_card_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    file_browser = (file_browser_s *)malloc(sizeof(file_browser_s));
    button_evt = (uint8_t *)malloc(BUTTON_NUM);

	memset(file_browser, 0, sizeof(file_browser_s));

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
		if(xQueueReceive(button_evt_queue, button_evt, 50/portTICK_PERIOD_MS) == pdTRUE)
		{
			if(button_evt[BUTTON_UP] == BUTTON_EVT_PRESSED_UP)
			{
			}

			if(button_evt[BUTTON_DOWN] == BUTTON_EVT_PRESSED_UP)
			{
			}

			if(button_evt[BUTTON_MIDDLE] == BUTTON_EVT_PRESSED_UP)
			{

			}
		}
    }
}

