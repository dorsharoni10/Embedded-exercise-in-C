#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "exercise.h"


// Statements on functions
void parsing(char *, uint32_t *, uint32_t*, char *, uint32_t *, uint32_t *);

int find_timing_ano(const uint32_t, uint32_t *, const int);
void find_behavioral_ano(const uint32_t, const uint32_t, const uint32_t, const uint32_t, const uint8_t, uint32_t *,
	uint32_t *, uint32_t *, uint8_t *, uint8_t  *, uint8_t *, unsigned int *, uint16_t *);
void find_correlation_ano(const uint32_t, const uint32_t, uint32_t *, const uint32_t, const uint32_t, const uint8_t,
	uint32_t *, uint32_t *, uint32_t *, unsigned int *, uint16_t *);

int timing_file_import(const char *, unsigned int *);
int behavioral_file_import(const char *, unsigned int *);
int correlation_file_import(const char *, unsigned int *);


int main()
{

	unsigned int anomalies_ids[1000] = { 0 };

	const char *path = "C:\\Users\\דור\\Desktop\\Embedded-exercise\\example_inputs\\correlation\\136_104183_197608_197623_197639_197641.txt";

	//correlation

	detect_correlation_anomalies(path, anomalies_ids);
	system("pause");

	//timing	
	//detect_timing_anomalies(path, anomalies_ids);

	//behavioral	
	//detect_behavioral_anomalies(path, anomalies_ids);

	return 0;
}




// take a line from the file and split to variables
void parsing(char *input_line, uint32_t *id, uint32_t *mil_sec, char *ECU_id, uint32_t *val1, uint32_t *val2)
{
	char *string = (char*)malloc(sizeof(char) * 100);

	//split the string
	string = strtok(input_line, " ");

	//parsing to id 
	*id = atoi(string);
	string = strtok(NULL, " ,.-");

	//parsing to mil_sec 
	*mil_sec = atoi(string);
	string = strtok(NULL, " ,.-");

	//parsing to ECU id 
	strcpy(ECU_id, string);
	string = strtok(NULL, " ,.-");

	//parsing to val1 
	*val1 = atoi(string);
	string = strtok(NULL, " ,.-");

	//parsing to val2 
	*val2 = atoi(string);

	//remove word from string
	string = strtok(NULL, " ,.-");

	free(string);
}


int find_timing_ano(const uint32_t mil_sec, uint32_t *prev, const int limit)
{
	if (mil_sec >= *prev + limit)
	{
		*prev = mil_sec;
	}
	else
	{	//if mil_sec < *prev + limit is anomaly
		*prev = mil_sec;
		return 1;
	}
	return -1;
}


void find_behavioral_ano(const uint32_t id, const uint32_t mil_sec, const uint32_t val1, const uint32_t val2, const uint8_t case_ECUid, uint32_t *prev_speed,
	uint32_t *prev_timing_pedal_1, uint32_t *prev_timing_pedal_2, uint8_t *flag_crash, uint8_t  *flag_pedal_1, uint8_t *flag_pedal_2, unsigned int *anomalies_ids, uint16_t *count_anomalies_ids)
{
	uint8_t flag_ano = 0;

	switch (case_ECUid)
	{
	case 0: //"0x100"

		// Make sure all the values are within the given range 
		if (val1 != 0 || val2 < 0 || val2>300)
		{
			anomalies_ids[(*count_anomalies_ids)++] = id;
		}

		// The car’s speed may not change faster than 5 kmh within 50 milliseconds

		else if ((int)val2 > (int)(*prev_speed + 5) && *flag_crash == 0)
		{
			anomalies_ids[(*count_anomalies_ids)++] = id;
		}

		else if ((int)val2 < (int)(*prev_speed - 5) && *flag_crash == 0)
		{
			//cheack is not a car crash
			if (val2 != 0)
			{
				anomalies_ids[(*count_anomalies_ids)++] = id;
			}
			else
			{
				//Any positive speed after there was a crash should be treated as an anomaly.
				//flag_crash = 1, for to dont cheack the speed again and again 

				*flag_crash = 1;
			}
		}
		*prev_speed = val2;
		break;

	case 1: //"0x200"

		// Make sure all the values are within the given range 
		if (val1 < 0 || val1>100 || val2 < 0 || val2>100)
		{
			anomalies_ids[(*count_anomalies_ids)++] = id;
			flag_ano = 1;
		}
		else
		{
			//The minimum time a pedal can be pressed(value1 > 0) is 10 milliseconds
			if (val1 == 0 && mil_sec - *prev_timing_pedal_1 < 10 && *flag_pedal_1 == 1 && flag_ano == 0)
			{
				flag_ano = 1;
				*flag_pedal_1 = 0;
				anomalies_ids[(*count_anomalies_ids)++] = id;
			}
			if (val1 == 0)
				*flag_pedal_1 = 0;

			if (val1 > 0 && *flag_pedal_1 == 0)
			{
				*prev_timing_pedal_1 = mil_sec;
				*flag_pedal_1 = 1;
			}

			//The minimum time a pedal can be pressed(value2 > 0) is 10 milliseconds
			if (val2 == 0 && mil_sec - *prev_timing_pedal_2 < 10 && *flag_pedal_2 == 1 && flag_ano == 0)
			{
				flag_ano = 1;
				*flag_pedal_2 = 0;
				anomalies_ids[(*count_anomalies_ids)++] = id;
			}
			if (val2 == 0)
				*flag_pedal_2 = 0;

			if (val2 > 0 && *flag_pedal_2 == 0)
			{
				*prev_timing_pedal_2 = mil_sec;
				*flag_pedal_2 = 1;
			}
		}

		//The gas and brake pedal cannot be pressed simultaneously
		if (val1 > 0 && val2 > 0 && flag_ano == 0)
			anomalies_ids[(*count_anomalies_ids)++] = id;

		break;

	case 2: // "0x400"

		// Make sure all the values are within the given range 
		if (val1 != 0 || val2 < 0 || val2>1)
			anomalies_ids[(*count_anomalies_ids)++] = id;
		break;

	case 3: // "0x800"

		// Make sure all the values are within the given range 
		if (val1 != 0 || val2 < 0 || val2>100)
			anomalies_ids[(*count_anomalies_ids)++] = id;
		break;
	}
}

void find_correlation_ano(const uint32_t id, const uint32_t val2, uint32_t *prv_speed, const uint32_t accleration, const uint32_t brake
	, const uint8_t case_ECUid, uint32_t *curr_tire, uint32_t *prv_tire, uint32_t *curr_speed, unsigned int *anomalies_ids, uint16_t *count_anomalies_ids)
{
	switch (case_ECUid)
	{
	case 0:
		//the last curr_speed
		*curr_speed = val2;


		//When brakes are active ( > 0), speed should not be increasing
		if (accleration > 0 && *prv_speed > *curr_speed)
			anomalies_ids[(*count_anomalies_ids)++] = id;

		else if (brake > 0 && *prv_speed < *curr_speed)
			anomalies_ids[(*count_anomalies_ids)++] = id;

		//When tire pressure is below 30, speed can’t be above 50 kmh. Speed above
		//50 kmh reported after tire was below 30 is considered an anomaly.

		else if (*curr_tire < 30 && *curr_speed > 50)
			anomalies_ids[(*count_anomalies_ids)++] = id;

		*prv_speed = *curr_speed;
		break;

	case 2:
		//When brake pedal is pressed hard(80 + ), ABS will activate
		if (brake >= 80 && val2 != 1)
			anomalies_ids[(*count_anomalies_ids)++] = id;
		break;
	case 3:

		*curr_tire = val2;

		//Tire pressure can’t increase while the car is moving
		if (*curr_speed > 0 && *prv_tire < *curr_tire)
			anomalies_ids[(*count_anomalies_ids)++] = id;

		//When tire pressure is below 30, speed can’t be above 50 kmh
		//Tire pressure below 30, when speed was above 50 is also considered an anomaly
		else if (*curr_tire < 30 && *curr_speed > 50)
			anomalies_ids[(*count_anomalies_ids)++] = id;

		*prv_tire = *curr_tire;
		break;
	}
}


int timing_file_import(const char *path, unsigned int *anomalies_ids)
{
	char input_line[100], ECU_id[6];
	const uint8_t line_size = 100;
	uint8_t is_anomaly = -1;
	uint16_t count_anomalies_ids = 0;
	uint32_t id = 0, mil_sec = 0, val1 = 0, val2 = 0, prv_speed = 0, prv_pedal = 0, prv_abs = 0, prv_tire = 0, count = 0;

	FILE *file = fopen(path, "r");
	if (!file)
	{
		return -1;
	}

	//reading line1
	while (fgets(input_line, line_size, file) != NULL)
	{

		parsing(input_line, &id, &mil_sec, ECU_id, &val1, &val2);

		//first 4 lines are initial values and can be skipped
		if (count > 3)
		{
			if (strcmp(ECU_id, "0x100") == 0)
			{
				//one message every 50 millisec.
				is_anomaly = find_timing_ano(mil_sec, &prv_speed, 50);

			}
			else if (strcmp(ECU_id, "0x200") == 0)
			{
				//one message every 5 millisec.
				is_anomaly = find_timing_ano(mil_sec, &prv_pedal, 5);
			}
			else if (strcmp(ECU_id, "0x400") == 0)
			{
				//one message every 10 millisec.
				is_anomaly = find_timing_ano(mil_sec, &prv_abs, 10);
			}
			else if (strcmp(ECU_id, "0x800") == 0)
			{
				//one message every 100 millisec.
				is_anomaly = find_timing_ano(mil_sec, &prv_tire, 100);
			}

			if (is_anomaly == 1)
				anomalies_ids[count_anomalies_ids++] = id;

			//cheack if get the max number of anomaly that ask for
			if (count_anomalies_ids >= 1000)
				return 0;
		}
		count++;
	}
	fclose(file);
	return 0;
}

int behavioral_file_import(const char *path, unsigned int *anomalies_ids)
{
	char input_line[100], ECU_id[6];
	const uint8_t line_size = 100;
	uint8_t  flag_crash = 0, flag_pedal_1 = 0, flag_pedal_2 = 0;
	uint16_t count_anomalies_ids = 0;
	uint32_t id = 0, mil_sec = 0, val1 = 0, val2 = 0, prev_speed = 0, prev_timing_pedal_1 = 0, prev_timing_pedal_2 = 0;

	FILE *file = fopen(path, "r");
	if (!file)
	{
		return -1;
	}
	//reading line
	while (fgets(input_line, line_size, file) != NULL)
	{
		parsing(input_line, &id, &mil_sec, ECU_id, &val1, &val2);

		if (strcmp(ECU_id, "0x100") == 0)
		{
			//cheack if have any positive speed after crash
			if (flag_crash == 1 && val2 > 0)
				anomalies_ids[count_anomalies_ids++] = id;
			else
				find_behavioral_ano(id, mil_sec, val1, val2, 0, &prev_speed, &prev_timing_pedal_1,
					&prev_timing_pedal_2, &flag_crash, &flag_pedal_1, &flag_pedal_2, anomalies_ids, &count_anomalies_ids);
		}
		else if (strcmp(ECU_id, "0x200") == 0)
		{
			find_behavioral_ano(id, mil_sec, val1, val2, 1, &prev_speed, &prev_timing_pedal_1,
				&prev_timing_pedal_2, &flag_crash, &flag_pedal_1, &flag_pedal_2, anomalies_ids, &count_anomalies_ids);
		}
		else if (strcmp(ECU_id, "0x400") == 0)
		{
			find_behavioral_ano(id, mil_sec, val1, val2, 2, &prev_speed, &prev_timing_pedal_1,
				&prev_timing_pedal_2, &flag_crash, &flag_pedal_1, &flag_pedal_2, anomalies_ids, &count_anomalies_ids);
		}
		else if (strcmp(ECU_id, "0x800") == 0)
		{
			find_behavioral_ano(id, mil_sec, val1, val2, 3, &prev_speed, &prev_timing_pedal_1,
				&prev_timing_pedal_2, &flag_crash, &flag_pedal_1, &flag_pedal_2, anomalies_ids, &count_anomalies_ids);
		}

		if (count_anomalies_ids >= 1000)
			return 0;

	}
	fclose(file);
	return 0;
}

int correlation_file_import(const char *path, unsigned int *anomalies_ids)
{

	char input_line[100], ECU_id[6];
	const uint8_t line_size = 100;
	uint16_t count_anomalies_ids = 0;
	uint32_t id = 0, val1 = 0, val2 = 0, brake = 0, accleration = 0, prv_speed = 0, curr_tire = 0, prv_tire = 0, curr_speed = 0, mil_sec = 0;


	FILE *file = fopen(path, "r");
	if (!file)
	{
		return -1;
	}
	//reading line1
	while (fgets(input_line, line_size, file) != NULL)
	{

		parsing(input_line, &id, &mil_sec, ECU_id, &val1, &val2);

		if (strcmp(ECU_id, "0x100") == 0)
		{
			find_correlation_ano(id, val2, &prv_speed, accleration, brake, 0, &curr_tire, &prv_tire, &curr_speed, anomalies_ids, &count_anomalies_ids);

		}
		else if (strcmp(ECU_id, "0x200") == 0)
		{
			//Take here the latest values
			accleration = val1;
			brake = val2;
		}
		else if (strcmp(ECU_id, "0x400") == 0)
		{
			find_correlation_ano(id, val2, &prv_speed, accleration, brake, 2, &curr_tire, &prv_tire, &curr_speed, anomalies_ids, &count_anomalies_ids);

		}
		else if (strcmp(ECU_id, "0x800") == 0)
		{
			find_correlation_ano(id, val2, &prv_speed, accleration, brake, 3, &curr_tire, &prv_tire, &curr_speed, anomalies_ids, &count_anomalies_ids);
		}

		if (count_anomalies_ids >= 1000)
			return 0;
	}
	fclose(file);
	return 0;
}


int detect_timing_anomalies(const char* file_path, unsigned int *anomalies_ids)
{
	int result = 0;

	if (!file_path)
		return -1;

	result = timing_file_import(file_path, anomalies_ids);

	return result;
}

int detect_behavioral_anomalies(const char* file_path, unsigned int *anomalies_ids)
{
	int result = 0;

	if (!file_path)
		return -1;

	result = behavioral_file_import(file_path, anomalies_ids);

	return result;
}


int detect_correlation_anomalies(const char* file_path, unsigned int *anomalies_ids)
{
	int result = 0;

	if (!file_path)
		return -1;

	result = correlation_file_import(file_path, anomalies_ids);

	return result;
}
