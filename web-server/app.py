from flask import Flask, request, jsonify
import telegram
# from telebot.credentials import bot_token, URL
# from telebot.mastermind import get_response
import db
import logging
import threading
import time
from tgbot import bot
logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

# bot = telegram.Bot(token=TOKEN)
app = Flask(__name__)
logger = logging.getLogger(__name__)

# @app.route("/{}".format(TOKEN), methods=["POST"])
# def respond():
#     print("Hi")
#     update = telegram.Update.de_json(request.get_json(force=True), bot)
#     chat_id = update.message.chat.id
#     msg_id = update.message.message_id
#     text = update.message.text.encode("utf-8").decode()
#     logger.info("got text message : "+text)
#     response = get_response(text,chat_id,update.message.from_user.first_name)
#     bot.sendMessage(chat_id=chat_id, text=response, reply_to_message_id=msg_id)
#     return "ok"


# def set_webhook():
#     # we use the bot object to link the bot to our app which live
#     # in the link provided by URL
#     s = bot.setWebhook("{URL}{HOOK}".format(URL=URL, HOOK=TOKEN),certificate=open('server.crt','rb'))
#     # something to let us know things work
#     if s:
#         return "Webhook setup ok"
#     else:
#         return "Webhook setup failed"


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


@app.route("/audio", methods=["POST"])
def saveaudio():
    audio = request.values.get("data")
    """ TODO
    Check access control value from db.
    If not allowed, then send command 3 to queue.
    Otherwise, send command 4 with username retrieved from db 
    """
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
        x = db.query_db('Select * from AccessFlag where time < now()-30m  and "control" = 1')
        if len(x) > 0:
            db.del_dball("AccessFlag")
            db.add_db("AccessFlag",{"control":0})
        y = db.query_db('Select * from ReminderLog where time > now()-30m')
        if len(y) == 0:
            db.del_dball("ReminderLog")


def unsafe_reset_all():
    db.del_dball("Humidity")
    db.del_dball("Usertb")
    db.del_dball("AccessFlag")
    db.del_dball("Command")
    db.del_dball("RegisteredAccess")
    db.del_dball("ReminderLog")

if __name__ == "__main__":
    unsafe_reset_all() #comment if needed
    thread = threading.Thread(target=threaded_control)
    thread.start()
    app.run(host='0.0.0.0',port=9876,threaded=True)

