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
logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

# bot = telegram.Bot(token=TOKEN)
app = Flask(__name__)
logger = logging.getLogger(__name__)

print(notelist,freqlist)
@app.route("/savehumidity", methods=["GET"])
def savehumid():
    humid = request.args.get("humidity")
    if humid is None:
        return "wrong request", 400
    try:
        humid = int(humid)
    except:
        return "must be integer", 400
    db.add_db("Humidity",{'humidity': humid})
    # If exceed threshold, send reminder (once every 30 mins)
    if humid > 10:
        y = db.query_db("Select * from ReminderLog")
        if len(y) == 0:
            db.add_db("ReminderLog",{"remind":"humid"})
            allusers = db.query_db('Select "chat_id" from Usertb')
            remindmsg = f"Please note that the humidity is now {humid}, please insert the humidity tube."
            for userx in allusers:
                chat_id = userx['chat_id']
                bot.sendMessage(chat_id,remindmsg)
    return "OK", 200


@app.route("/access", methods=["POST"])
def saveaccess():
    accesscode = request.values.get("id")
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
        db.add_db("AccessRecord",{"name": uname})
        db.add_db("Command",{"command":1})
        remindmsg = f"{uname} is accessing the piano."
    else:
        db.add_db("Command",{"command":2})
        db.add_db("AccessRecord",{"name": "Unidentified (blocked)"})
        db.del_dball("AccessFlag")
        db.add_db("AccessFlag",{"control":0})
        remindmsg = f"An unidentified card has requested access to the piano and is blocked. If it should be allowed, please use the /register command."
    allusers = db.query_db('Select "chat_id" from Usertb')
    for userx in allusers:
        chat_id = userx['chat_id']
        bot.sendMessage(chat_id,remindmsg)
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
    while True:
        time.sleep(5)
        #Clear Reminder Log to allow new reminders
        y = db.query_db('Select * from ReminderLog where time > now()-30m')
        if len(y) == 0:
            db.del_dball("ReminderLog")
        #Check if being accessed
        z = db.query_db('Select mean("frequency"),stddev("frequency") from Frequency where time > now()-30s')
        if len(z) == 0:
            fmean,fsd = None,None
        else:
            fmean,fsd = z[0]['mean'],z[0]['stddev']
        if fmean is not None and fsd is not None and fmean > 200 and fsd > 100:
            #is_playing
            in_use = db.query_db('Select * from InuseFlag')[0]['control']
            if in_use == 0:
                db.del_dball("InuseFlag")
                db.add_db("InuseFlag",{"control":1})
                db.add_db("UseRecord",{"start_use":1})
                access_control = db.query_db('Select * from AccessFlag')[0]['control']
                if access_control == 0:
                    db.add_db("Command",{'command':3})
                else:
                    cuser = db.query_db('Select name from AccessRecord GROUP BY * ORDER BY DESC LIMIT 1')[0]['name']
                    db.add_db("Command",{'command':4,'name': cuser})
        else:
            #not_playing
            in_use = db.query_db('Select * from InuseFlag')[0]['control']
            if in_use == 1:
                db.del_dball("InuseFlag")
                db.add_db("InuseFlag",{"control":0})
                db.add_db("UseRecord",{"start_use":0})
        # Check if tuning mode is opened.
        r = db.query_db('Select mean("frequency"),stddev("frequency") from Frequency where time > now()-5s')
        if len(r) == 0:
            tunemean,tunesd = None,None
        else:
            tunemean,tunesd = r[0]['mean'],r[0]['stddev']
        if tunemean is not None and tunesd is not None and tunemean >= freqlist[0] and tunemean < freqlist[-1] and tunesd < 20:
            for i,freq in enumerate(freqlist):
                if tunemean >= freq:
                    tempindex = i
                    break
            correctindex = tempindex if abs(tunemean-freqlist[tempindex]) < abs(tunemean-freqlist[tempindex+1]) else tempindex + 1
            accepted_deviation = (freqlist[tempindex+1]-freqlist[tempindex])/4
            if abs(tunemean-freqlist[correctindex]) > accepted_deviation:
                db.add_db("TuneReminders",{"note":notelist[correctindex],"detected_frequency":tunemean,"correct_frequency":freqlist[correctindex]})
                msg = f"Tune Reminder: Detected Frequency {tunemean:.2f}Hz for note {notelist[correctindex]} while the correct frequency should be {freqlist[correctindex]}Hz. A tuning service will be scheduled for you."
            else:
                msg = f"The note {notelist[correctindex]}'s detected frequency is {truemean:.2f}Hz, which is within acceptable deviation with the true frequency {freqlist[correctindex]}Hz."
            allusers = db.query_db('Select "chat_id" from Usertb')
            for userx in allusers:
                chat_id = userx['chat_id']
                bot.sendMessage(chat_id,msg)
        # Revoke access since time has passed
        x = db.query_db('Select * from AccessFlag where time < now()-30m  and "control" = 1')
        if len(x) > 0:
            db.del_dball("AccessFlag")
            db.add_db("AccessFlag",{"control":0})

def unsafe_reset_all():
    db.del_dball("Humidity")
    db.del_dball("Usertb")
    db.del_dball("AccessFlag")
    db.del_dball("Command")
    db.del_dball("RegisteredAccess")
    db.del_dball("ReminderLog")
    db.del_dball("InuseFlag")
    db.del_dball("Frequency")
    db.del_dball("UseRecord")
    db.del_dball("Tuning")
    db.del_dball("TuneReminders")
    db.add_db("InuseFlag",{"control":0})
    db.add_db("AccessFlag",{"control":0})

if __name__ == "__main__":
    unsafe_reset_all() #comment if needed
    thread = threading.Thread(target=threaded_control)
    thread.start()
    app.run(host='0.0.0.0',port=9876,threaded=True)

