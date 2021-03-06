from flask import Flask, request, jsonify
import telegram
# from telebot.credentials import bot_token, URL
# from telebot.mastermind import get_response
import db
import logging
import threading
import time
from tgbot import bot
from frequencies import notelist,freqlist
import threading
logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

# bot = telegram.Bot(token=TOKEN)
app = Flask(__name__)
logger = logging.getLogger(__name__)
global access_user
global is_cancelled
last_access_time = time.time()
is_cancelled = False
begin = True

def printf(s):
    s = str(s)
    with open("debug.txt","a") as f:
        f.write(s)
        f.write("\n")

def telegram_announcement(msg):
    allusers = db.query_db('Select "chat_id" from Usertb')
    for userx in allusers:
        chat_id = userx['chat_id']
        bot.sendMessage(chat_id,msg)

# def delayed_announcement(msg,flag,cuser):
#     time.sleep(5)
#     logger.info("HIHIHIHI")
#     if not is_cancelled:
#         telegram_announcement(msg)
#         if flag == 1:
#             db.add_db("Command",{'command':3})
#         else:
#             db.add_db("Command",{'command':4,'uname': cuser})



@app.route("/savehumidity", methods=["GET"])
def savehumid():
    humid = request.args.get("humidity")
    if humid is None:
        return "wrong request", 400
    try:
        humid = float(humid)
    except:
        return "must be integer", 400
    db.add_db("Humidity",{'humidity': humid})
    # If exceed threshold, send reminder (once every 30 mins)
    if humid > 60:
        y = db.query_db("Select * from ReminderLog")
        if len(y) == 0:
            db.add_db("ReminderLog",{"remind":"humid"})
            remindmsg = f"Please note that the humidity is now {humid}, please insert the humidity tube."
            telegram_announcement(remindmsg)
    return "OK", 200


@app.route("/access", methods=["GET"])
def saveaccess():
    accesscode = request.args.get("id")
    logger.info(str(accesscode))
    allowedlist = db.query_db("Select * from RegisteredAccess")
    is_allowed = False
    uname = None
    for allowed in allowedlist:
        if allowed['code'] == accesscode:
            is_allowed = True
            uname = allowed['name']
            break
    if is_allowed:
        db.del_dball("AccessFlag")
        db.add_db("AccessFlag",{"control":1})
        db.add_db("AccessRecord",{"uname": uname})
        db.add_db("Command",{"command":1})
        remindmsg = f"{uname} is accessing the piano."
    else:
        db.add_db("Command",{"command":2})
        db.add_db("AccessRecord",{"uname": "Unidentified (blocked)"})
        db.del_dball("AccessFlag")
        db.add_db("AccessFlag",{"control":0})
        remindmsg = f"An unidentified card has requested access to the piano and is blocked. If it should be allowed, please use the /register command."
    telegram_announcement(remindmsg)
    return "OK", 200


@app.route("/audio", methods=["GET"])
def saveaudio():
    audio = request.args.get("frequency")
    if audio is None:
        return "wrong request",400
    try:
        audio = float(audio)
    except:
        return "must be a float value", 400
    db.add_db("Frequency",{"frequency":audio})
    #Further process is done in threaded control, because this endpoint is called too often.
    return "OK", 200


@app.route("/command", methods=["GET"])
def cmdok():
    cmd = db.query_db('Select * from Command GROUP BY * ORDER BY DESC LIMIT 1')
    if len(cmd) > 0:
        cmd = cmd[0]
        db.del_dball("Command")
        return jsonify({"Status":"OK","Data": cmd})
    else:    
        return jsonify({"Status": "NOCMD"})


@app.route("/")
def index():
    return jsonify({"message": "Hello,hello, World!"})

def threaded_control():
    global last_access_time
    global begin
    access_user = "placeholder"
    while True:
        time.sleep(5)
        #Clear Reminder Log to allow new reminders
        y = db.query_db('Select * from ReminderLog where time > now()-1m')
        if len(y) == 0:
            db.del_dball("ReminderLog")
        #Check if being accessed
        z = db.query_db('Select frequency from Frequency where time > now()-20s')
        r = db.query_db('Select stddev("frequency") from Frequency where time > now()-10s')
        try:
            r = r[0]['stddev']
        except:
            r = 0
        count = 0
        for records in z:
            if records['frequency'] >= 200:
                count += 1
        printf("count"+str(count)+" "+str(len(z)))
        if (len(z) >0 and count/len(z) > 0.6) or r > 400:
            #is_playing
            in_use = db.query_db('Select * from InuseFlag')[0]['control']
            printf("playing!")
            db.add_db("UseRecord",{"playing":1})
            if in_use == 0:
                last_access_time = time.time()
                db.del_dball("InuseFlag")
                db.add_db("InuseFlag",{"control":1})
                access_control = db.query_db('Select * from AccessFlag')[0]['control']
                printf("access")
                printf(access_control)
                if access_control == 0:
                    printf("hi2")
                    is_cancelled = False
                    msg = "Someone is playing on the piano without proper access!"
                    telegram_announcement(msg)
                    db.add_db("Command",{'command':3})
                    # threading.Thread(target=delayed_announcement,args=(msg,1,"asd")).start()
                    access_user = "unauthorized"
                else:
                    printf("hihi")
                    cuser = db.query_db('Select uname from AccessRecord GROUP BY * ORDER BY DESC LIMIT 1')[0]['uname']
                    access_user = cuser
                    is_cancelled = False
                    msg = f"The piano is being used by {cuser} now."
                    telegram_announcement(msg)
                    db.add_db("Command",{'command':4,'uname': cuser})
                    # threading.Thread(target=delayed_announcement,args=(msg,2,cuser)).start()
        else:
            #not_playing
            printf("goodlordhihi")
            db.add_db("UseRecord",{"playing":0})
            in_use = db.query_db('Select * from InuseFlag')[0]['control']
            if in_use == 1:
                if begin:
                    db.del_dball("InuseFlag")
                    db.add_db("InuseFlag",{"control":0})
                else:
                    is_cancelled = True
                    useduration = (time.time() - last_access_time)/60
                    telegram_announcement("The piano is now unoccupied.")
                    db.add_db("UseDuration",{"piano_user":access_user,"duration":useduration})
                    db.del_dball("InuseFlag")
                    db.add_db("InuseFlag",{"control":0})
                    db.add_db("Command",{'command':5})
        # Check if tuning mode is opened.
        r = db.query_db('Select mean("frequency"),stddev("frequency") from Frequency where time > now()-3s')
        if len(r) == 0:
            tunemean,tunesd = None,None
        else:
            tunemean,tunesd = r[0]['mean'],r[0]['stddev']
        printf("Tunemean "+str(tunemean))
        printf("Tunesd "+str(tunesd))
        if tunemean is not None and tunesd is not None and tunemean >= freqlist[0] and tunemean < freqlist[-1] and tunesd < 20:
            printf("tuninghere")
            for i in range(len(freqlist)-1):
                if freqlist[i]<= tunemean and tunemean < freqlist[i+1]:
                    tempindex = i 
                    break
            printf(tempindex)
            correctindex = tempindex if abs(tunemean-freqlist[tempindex]) < abs(tunemean-freqlist[tempindex+1]) else tempindex + 1
            accepted_deviation = (freqlist[tempindex+1]-freqlist[tempindex])/4
            printf(correctindex)
            dev = abs(tunemean-freqlist[correctindex])
            if  dev > accepted_deviation:
                db.add_db("TuneReminders",{"note":notelist[correctindex],"detected_frequency":tunemean,"correct_frequency":freqlist[correctindex]})
                msg = f"Tune Reminder: Detected Frequency {tunemean:.2f}Hz for note {notelist[correctindex]} while the correct frequency should be {freqlist[correctindex]}Hz. A tuning service will be scheduled for you."
            else:
                msg = f"The note {notelist[correctindex]}'s detected frequency is {tunemean:.2f}Hz, which is within acceptable deviation with the true frequency {freqlist[correctindex]}Hz."
            telegram_announcement(msg)
            db.add_db("Frequency",{"deviation": dev})
        # Revoke access since time has passed
        x = db.query_db('Select * from AccessFlag where time < now()-30m  and "control" = 1')
        if len(x) > 0:
            db.del_dball("AccessFlag")
            db.add_db("AccessFlag",{"control":0})
        begin = False

def unsafe_reset_all():
    db.del_dball("Humidity")
    db.del_dball("Usertb")
    db.del_dball("AccessFlag")
    db.del_dball("Command")
    db.del_dball("RegisteredAccess")
    db.del_dball("ReminderLog")
    db.del_dball("AccessRecord")
    db.del_dball("InuseFlag")
    db.del_dball("Frequency")
    db.del_dball("UseRecord")
    db.del_dball("UseDuration")
    db.del_dball("Tuning")
    db.del_dball("TuneReminders")
    db.add_db("InuseFlag",{"control":0})
    db.add_db("AccessFlag",{"control":0})

if __name__ == "__main__":
    # unsafe_reset_all() #comment if needed
    thread = threading.Thread(target=threaded_control)
    thread.start()
    app.run(host='0.0.0.0',port=9876,threaded=True)

