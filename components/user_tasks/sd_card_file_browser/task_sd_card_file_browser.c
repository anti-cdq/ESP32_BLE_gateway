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
#include "multi_task_management.h"

static const char *TAG = "SD CARD";


#define FILE_PATH_LEN			50
#define FILE_NUM_PER_PAGE		12
#define FILE_BUFF_LEN			1024
#define LCD_BUFF_LEN			(LCD_W*2)

#define SDMMC_HOST_USER() {\
    .flags = SDMMC_HOST_FLAG_4BIT, \
    .slot = SDMMC_HOST_SLOT_1, \
    .max_freq_khz = SDMMC_FREQ_HIGHSPEED, \
    .io_voltage = 3.3f, \
    .init = &sdmmc_host_init, \
    .set_bus_width = &sdmmc_host_set_bus_width, \
    .get_bus_width = &sdmmc_host_get_slot_width, \
    .set_card_clk = &sdmmc_host_set_card_clk, \
    .do_transaction = &sdmmc_host_do_transaction, \
    .deinit = &sdmmc_host_deinit, \
    .io_int_enable = sdmmc_host_io_int_enable, \
    .io_int_wait = sdmmc_host_io_int_wait, \
    .command_timeout_ms = 0, \
}

typedef struct
{
	uint8_t display_flag;
	uint8_t status;
	uint32_t path_depth;
	uint32_t file_num;
	uint32_t page_num;
	uint32_t page_file_num;
	uint32_t page_index_p;
	uint32_t page_index_c;
	uint32_t file_index_p;
	uint32_t file_index_c;
	uint32_t file_index_min;
	uint32_t file_index_max;
	uint8_t	Buff[LCD_W*3];
	uint8_t buff_to_lcd[LCD_BUFF_LEN];
	char current_path[FILE_PATH_LEN];
	char all_file_name[FILE_NUM_PER_PAGE][FILE_PATH_LEN];
	uint8_t filetype[FILE_NUM_PER_PAGE];
	uint8_t button_evt[BUTTON_NUM];
}file_browser_s;

file_browser_s *file_browser;


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


uint32_t get_file_num(char* path)
{
	DIR * dir;
	struct dirent * ptr;
	dir = opendir(path);
	uint32_t file_num = 0;

	while((ptr = readdir(dir)) != NULL)
	{
		file_num++;
	}
	closedir(dir);
	return file_num;
}


void get_file_name(char* path, uint32_t start_index)
{
	uint32_t i;
	DIR * dir;
	struct dirent * ptr;
	dir = opendir(path);
	uint32_t file_index = 0;

	while((ptr = readdir(dir)) != NULL)
	{
		file_index++;
		if(file_index >= start_index+FILE_NUM_PER_PAGE)
			break;
		if(file_index >= start_index)
		{
			memset(file_browser->all_file_name[file_index-start_index], 0, FILE_PATH_LEN);
			memcpy(	file_browser->all_file_name[file_index-start_index],
					ptr->d_name,
					strlen(ptr->d_name));
		}
		file_browser->filetype[file_index-start_index] = ptr->d_type;
	}
	closedir(dir);
}


void mem_free_task_sd_card_file_browser(void)
{
	// All done, unmount partition and disable SDMMC or SPI peripheral
	esp_vfs_fat_sdmmc_unmount();
	ESP_LOGI(TAG, "Card unmounted");

	free(file_browser);
}


void lcd_display_task_sd_card_file_browser(void)
{
	uint32_t i = 0;
	char temp[9];

	if(file_browser == NULL)
		return;
	if(file_browser->display_flag == 1)
	{
		show_bmp_center(file_browser->current_path, 0, 0);
	}
	else if(file_browser->display_flag == 2)
	{
		LCD_Clear(BLACK);
		memset(temp, 0, sizeof(temp));
		LCD_ShowString(0, 0, (const uint8_t *)file_browser->current_path);
		sprintf(temp,
				"  %3d/%-3d",
				file_browser->page_index_c+1,
				file_browser->page_num);
		LCD_ShowString(160, 0, (const uint8_t *)temp);

		for(i=0;i<file_browser->page_file_num;i++)
		{
			LCD_ShowString(30, i*18+18, (const uint8_t *)file_browser->all_file_name[i]);
		}
		file_browser->display_flag = 0;
		file_browser->file_index_p = file_browser->file_index_max;
		file_browser->file_index_c = file_browser->file_index_min;
	}

	if(file_browser->file_index_c != file_browser->file_index_p)
	{
		LCD_ShowString(	15,
						(file_browser->file_index_p%FILE_NUM_PER_PAGE)*18 + 18,
						(const uint8_t *)" ");
		LCD_ShowString(	15,
						(file_browser->file_index_c%FILE_NUM_PER_PAGE)*18+18,
						(const uint8_t *)">");

		file_browser->file_index_p = file_browser->file_index_c;
	}
}


void update_to_path(char* path)
{
    file_browser->file_num = get_file_num(path);
    file_browser->page_num = file_browser->file_num/FILE_NUM_PER_PAGE;
    if(file_browser->file_num%FILE_NUM_PER_PAGE != 0)
    	file_browser->page_num++;
    file_browser->page_index_c = 0;
    file_browser->page_index_p = 1;
}


void task_sd_card_file_browser(void *pvParameter)
{
    ESP_LOGI(TAG, "Initializing SD card");
    ESP_LOGI(TAG, "Using SDMMC peripheral");
    sdmmc_host_t host = SDMMC_HOST_USER();
    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

    file_browser = (file_browser_s *)malloc(sizeof(file_browser_s));

	memset(file_browser, 0, sizeof(file_browser_s));
	memcpy(file_browser->current_path, "/sdcard", FILE_PATH_LEN);
	file_browser->display_flag = 0;

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
    update_to_path(file_browser->current_path);

    while(1)
    {
		if(xQueueReceive(button_evt_queue, file_browser->button_evt, 10/portTICK_PERIOD_MS) == pdTRUE)
		{
			if((file_browser->button_evt[BUTTON_UP]&BUTTON_EVT_MASK) == BUTTON_EVT_SINGLE_CLICK)
			{
				file_browser->file_index_c--;
			}
			if((file_browser->button_evt[BUTTON_DOWN]&BUTTON_EVT_MASK) == BUTTON_EVT_SINGLE_CLICK)
			{
				file_browser->file_index_c++;
			}
			if(file_browser->button_evt[BUTTON_UP] == BUTTON_EVT_HOLD_DOWN)
			{
				file_browser->file_index_c--;
			}
			if(file_browser->button_evt[BUTTON_DOWN] == BUTTON_EVT_HOLD_DOWN)
			{
				file_browser->file_index_c++;
			}
			if(file_browser->file_index_c == (file_browser->file_index_min - 1))
				file_browser->file_index_c = file_browser->file_index_max;
			if(file_browser->file_index_c > file_browser->file_index_max)
				file_browser->file_index_c = file_browser->file_index_min;

			if((file_browser->button_evt[BUTTON_LEFT]&BUTTON_EVT_MASK) == BUTTON_EVT_SINGLE_CLICK)
			{
				file_browser->page_index_c--;
			}
			if((file_browser->button_evt[BUTTON_RIGHT]&BUTTON_EVT_MASK) == BUTTON_EVT_SINGLE_CLICK)
			{
				file_browser->page_index_c++;
			}
			if(file_browser->page_index_c == 0xFFFFFFFF)
				file_browser->page_index_c = file_browser->page_num - 1;
			if(file_browser->page_index_c == file_browser->page_num)
				file_browser->page_index_c = 0;

			if((file_browser->button_evt[BUTTON_MIDDLE]&BUTTON_EVT_MASK) == BUTTON_EVT_SINGLE_CLICK)
			{
				if(file_browser->filetype[file_browser->file_index_c] == DT_DIR)
				{
					sprintf(file_browser->current_path,
							"%.*s%.*s%.*s",
							strlen(file_browser->current_path),
							file_browser->current_path,strlen("/"), "/",
							strlen(file_browser->all_file_name[file_browser->file_index_c]),
							file_browser->all_file_name[file_browser->file_index_c]);
					update_to_path(file_browser->current_path);
					file_browser->path_depth++;
				}
			}

			if((file_browser->button_evt[BUTTON_BACK]&BUTTON_EVT_MASK) == BUTTON_EVT_SINGLE_CLICK)
			{
				if(file_browser->path_depth)
				{
					file_browser->path_depth--;
					for(uint32_t i=FILE_PATH_LEN-1;i>0;i--)
					{
						if(file_browser->current_path[i] == '/')
						{
							file_browser->current_path[i] = 0;
							break;
						}
						file_browser->current_path[i] = 0;
					}
					update_to_path(file_browser->current_path);
				}
				else
				{
					user_task_disable();
				}
			}
		}

		if(file_browser->page_index_c != file_browser->page_index_p)
		{
		    if(file_browser->page_index_c < (file_browser->page_num - 1))
		    	file_browser->page_file_num = FILE_NUM_PER_PAGE;
		    else
		    {
		    	file_browser->page_file_num = file_browser->file_num%FILE_NUM_PER_PAGE;
		    	if(file_browser->page_file_num == 0)
			    	file_browser->page_file_num = FILE_NUM_PER_PAGE;
		    }
			file_browser->file_index_min = file_browser->page_index_c * FILE_NUM_PER_PAGE;
			file_browser->file_index_max = file_browser->page_index_c * FILE_NUM_PER_PAGE + file_browser->page_file_num - 1;
		    get_file_name(file_browser->current_path, file_browser->file_index_min + 1);
			file_browser->page_index_p = file_browser->page_index_c;
			file_browser->display_flag = 2;
		}
    }
}

