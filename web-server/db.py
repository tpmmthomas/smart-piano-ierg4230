import time
from influxdb import InfluxDBClient
import random

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
    '''
        Returns a list of dictionary, in which all the fields inserted as well as the time will be included.
    '''
    result = client.query(query)
    return list(result.get_points())


# result = client.query("Delete from Humidity where time >= '2021-12-11T04:03:53.334673Z' and time <= '2021-12-11T04:03:53.334675Z'")
# result = client.query("select * from Humidity")
# print(list(result.get_points()))
# # CAN ONLY DELETE BY TIME, but we can write auto thing to do such thing
# for i in range(1000):
#     x = random.randint(1,10)
#     data = [
#         {
#             "measurement": "Humidity",
#             "fields":{
#                 "humidity" : x
#             }
#         }
#     ]
#     client.write_points(data)
#     result = client.query("select * from Humidity")
#     print(list(result.get_points()))
#     time.sleep(5)



# from app import app

# DATABASE = "iot.db"


# def get_db():
#     db = getattr(g, "_database", None)
#     if db is None:
#         db = g._database = sqlite3.connect(DATABASE)
#     db.row_factory = sqlite3.Row
#     return db


# def query_db(query, args=(), one=False):
#     cur = get_db().execute(query, args)
#     rv = cur.fetchall()
#     cur.close()
#     return (rv[0] if rv else None) if one else rv

# def modify_db(query):
#     conn = get_db()
#     cur = conn.cursor()
#     cur.execute(query)
#     conn.commit()
#     conn.close()
    

# def init_db(app):
#     with app.app_context():
#         db = get_db()
#         with app.open_resource("schema.sql", mode="r") as f:
#             db.cursor().executescript(f.read())
#         db.commit()
