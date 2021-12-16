import time
from influxdb import InfluxDBClient
import random
import dateutil.parser
from datetime import timedelta

host = 'localhost'
port = 8086
uname = 'admin'
pw = '1234'
database = "measurements"
client = InfluxDBClient(host,port,uname,pw,database)

def add_db(measurement, fields):
    assert isinstance(measurement,str)
    assert isinstance(fields,dict)
    data = [
        {
            "measurement": measurement,
            "fields": fields
        }
    ]
    client.write_points(data)
    return

def del_db(measurement,times):
    #TODO: parse times for deletion, add one and minus one in the least significant place
    for timex in times:
        timex = dateutil.parser.isoparse(timex)
        timeb = timex + timedelta(microseconds = -1)
        timea = timex + timedelta(microseconds = 1)
        timeb = timeb.isoformat().split("+")[0]+"Z"
        timea = timea.isoformat().split("+")[0]+"Z"
        querystring = f"DELETE FROM {measurement} WHERE time > '{timeb}' and time < '{timea}'"
        client.query(querystring)
    return

def del_dball(measurement):
    client.query(f"Delete from {measurement}")
    return
    
def query_db(query):
    result = client.query(query)
    return list(result.get_points())

 
x = query_db('Select * from Humidity where time < now()-30m and "humidity" = 981')
print(x)

# timex = x[0]['time']
# measurement = "Humidity"
# timex = dateutil.parser.isoparse(timex)
# timeb = timex + timedelta(microseconds = -1)
# timea = timex + timedelta(microseconds = 1)
# timeb = timeb.isoformat().split("+")[0]+"Z"
# timea = timea.isoformat().split("+")[0]+"Z"
# querystring = f"DELETE FROM {measurement} WHERE time > '{timeb}' and time < '{timea}'"
# client.query(querystring)
# x = query_db('Select * from Humidity GROUP BY * ORDER BY DESC LIMIT 1')
# print(x)
# # y = dateutil.parser.isoparse(xtime)
# print(y)
# z = y + timedelta(microseconds = -1)
# z = z.isoformat().split("+")[0]+"Z"
# x = query_db('Select * from Humidity WHERE time >= ' + "'"+ z + "'")
# print(x)
# 'Select * from Humidity GROUP BY * ORDER BY DESC LIMIT 1' get last value
#Select * from Humidity WHERE "time" > \'2021-12-15T14:51:51.294900Z\' query by time in iso format
