import requests
import random
import time

base_url = "http://34.150.76.100:9876"
endpoint = "savehumidity"
field = "humidity"
minval = 0
maxval = 1000
current = 200

while True:
    current += random.randint(-10,10)
    current = min(maxval,current)
    current = max(minval,current)
    r = requests.get(base_url+"/"+endpoint+"?"+field+"="+str(current))
    print(r.status_code,current)
    time.sleep(5)