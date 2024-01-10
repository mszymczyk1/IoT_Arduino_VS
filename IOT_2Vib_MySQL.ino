#include <Ethernet.h>
#include <MySQL_Connection.h>
#include <MySQL_Cursor.h>

byte mac_addr[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

static const char user_passwd[] = "iot2023_device";               // MySQL user login username and password
int i = 0;
int v_0_max = 0;
int v_1_max = 0; 
int id_dev_0 = 0;
int id_dev_1 = 1;
const int threshold_v_default = 30;
int threshold_v_0 = threshold_v_default;
int threshold_v_1 = threshold_v_default;
char buff[100];

const char INSERT_TO_MYSQL[] = "INSERT INTO iot_project.%s (%s) VALUES (%s)";
const char SELECT_FROM_MYSQL[] = "SELECT id_device,device_attributes FROM iot_project.devices WHERE device_name LIKE 'vs%'";
const char UPDATE_IN_MYSQL[] = "UPDATE iot_project.devices SET active=1 WHERE id_device=%d";
const char dev[] = "devices";
const char dev_col[] = "device_name,serial_number,device_type,active,device_password"; // odswiezanie update_date
const char dev_value_0[] = "'vs0','a1x3','3','1','x3c5'";
const char dev_value_1[] = "'vs1','rvc5','3','1','dyh6'";

IPAddress server_ip(20,107,176,118);
EthernetClient client;
MySQL_Connection conn((Client *)&client);
MySQL_Cursor *cur_mem;

void display_freeram() {
  Serial.print(F("SRAM:"));
  Serial.print(freeRam());
}

int freeRam() {
  extern int __heap_start,*__brkval;
  int v;
  return (int)&v - (__brkval == 0  ? (int)&__heap_start : (int) __brkval);  
}


void setup() 
{
  Serial.begin(9600);
  Serial.println("\n\nMS");
  while (!Serial); // wait for serial port to connect
  if (Ethernet.begin(mac_addr) == 0) 
  {
    while (true) 
    {
      delay(1);
    }
  }
  
  Serial.print(F("IP:")); //server IP
  Serial.println(server_ip);
  display_freeram();
  Serial.println(F("\nCon"));

  if (conn.connect(server_ip, 3306, user_passwd, user_passwd)) 
  {
    delay(1000);
    cur_mem = new MySQL_Cursor(&conn);
    delay(1000);
    int res = cur_mem->execute(SELECT_FROM_MYSQL);

    if (!res) 
    {
      Serial.println(F("E3")); 
    } 
    else 
    { 
      column_names *columns = cur_mem->get_columns();
      row_values *row = NULL;
      row = cur_mem->get_next_row();

      if(row == NULL)
      {
        sprintf(buff, INSERT_TO_MYSQL, dev, dev_col, dev_value_0);
        Serial.println(buff);
        cur_mem->execute(buff);
        sprintf(buff, INSERT_TO_MYSQL, dev, dev_col, dev_value_1);
        Serial.println(buff);
        cur_mem->execute(buff);
        res = cur_mem->execute(SELECT_FROM_MYSQL);
        columns = cur_mem->get_columns();
        row = cur_mem->get_next_row();
        id_dev_0 = atoi(row->values[0]);
        row = cur_mem->get_next_row();
        id_dev_1 = atoi(row->values[0]);
        Serial.println(id_dev_0);
        Serial.println(id_dev_1);       
      }
      else
      {
        id_dev_0 = atoi(row->values[0]);
        threshold_v_0 = atoi(row->values[1]);

        if(threshold_v_0 == 0) 
        {
          threshold_v_0 = threshold_v_default;
        } 
        row = cur_mem->get_next_row();
        id_dev_1 = atoi(row->values[0]);
        threshold_v_1 = atoi(row->values[1]);

        if(threshold_v_1 == 0) 
        {
          threshold_v_1 = threshold_v_default;
        }
        Serial.println(id_dev_0);
        Serial.println(id_dev_1);
        Serial.println(threshold_v_0);
        Serial.println(threshold_v_1);
      }
    }
  }
  else
  {
    while (true) 
    {
      delay(1);
    }
  }
display_freeram();
Serial.println();
delete cur_mem;
cur_mem = NULL;
display_freeram();
Serial.println();
}


// Main loop 
void loop() 
{
  if(cur_mem == NULL)
  {
    cur_mem = new MySQL_Cursor(&conn);
  }
  display_freeram();
  Serial.print(F(" i="));
  Serial.print(i,DEC);
  char buff_2[25];
  int i_read = 0;
  int v_0;
  v_0=analogRead(0);//Connect the sensor to analog pin 0
  int v_1;
  v_1=analogRead(1);//Connect the sensor to analog pin 1
  Serial.print(F(" v0="));
  Serial.print(v_0,DEC);
  Serial.print(F(" v1="));
  Serial.println(v_1,DEC);

  if(v_0 >= threshold_v_0)
  {
    sprintf(buff_2, "'%d','active','%d'",v_0, id_dev_0);
    sprintf(buff, INSERT_TO_MYSQL, "alarms", "alarm_message,alarm_status,id_device", buff_2);
    Serial.println(buff);
    cur_mem->execute(buff);
  }

  if(v_1 >= threshold_v_1)
  {
    sprintf(buff_2, "'%d','active','%d'",v_1, id_dev_1);
    sprintf(buff, INSERT_TO_MYSQL, "alarms", "alarm_message,alarm_status,id_device", buff_2);
    Serial.println(buff);
    cur_mem->execute(buff);
  }

  if(i==30)
  {
    sprintf(buff_2, "'%d','%d'", v_0_max, id_dev_0);
    sprintf(buff, INSERT_TO_MYSQL, "logs", "log_content,id_device", buff_2);
    Serial.println(buff);
    cur_mem->execute(buff);
    sprintf(buff_2, "'%d','%d'", v_1_max, id_dev_1);
    sprintf(buff, INSERT_TO_MYSQL, "logs", "log_content,id_device", buff_2);
    Serial.println(buff);
    cur_mem->execute(buff);
    v_0_max = 0;
    v_1_max = 0;
    i = 0;
    //Update active
    sprintf(buff, UPDATE_IN_MYSQL, id_dev_0);
    Serial.println(buff);
    cur_mem->execute(buff);
    sprintf(buff, UPDATE_IN_MYSQL, id_dev_1);
    Serial.println(buff);
    cur_mem->execute(buff);
  }
  else
  {
    i++;

    if(v_0_max < v_0)
    {
      v_0_max = v_0;
    }

    if(v_1_max < v_1)
    {
      v_1_max = v_1;
    }

    delay(1000);    
  }
  i_read = Serial.read();

  if(i_read != -1)
  {
    conn.close();
    delete cur_mem;

    Serial.println("Cld");

    while(true)
    {
      delay(1);
    }
  }
}
