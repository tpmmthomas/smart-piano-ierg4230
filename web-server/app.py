from flask import Flask, request, jsonify
import telegram
from telebot.credentials import bot_token, URL
from telebot.mastermind import get_response
import db
import logging

logging.basicConfig(
    format="%(asctime)s - %(name)s - %(levelname)s - %(message)s", level=logging.INFO
)

global bot
global TOKEN
TOKEN = bot_token
bot = telegram.Bot(token=TOKEN)
app = Flask(__name__)
logger = logging.getLogger(__name__)

@app.route("/{}".format(TOKEN), methods=["POST"])
def respond():
    update = telegram.Update.de_json(request.get_json(force=True), bot)
    chat_id = update.message.chat.id
    msg_id = update.message.message_id
    text = update.message.text.encode("utf-8").decode()
    print("got text message :", text)
    response = get_response(text)
    bot.sendMessage(chat_id=chat_id, text=response, reply_to_message_id=msg_id)
    return "ok"


@app.route("/setwebhook", methods=["GET", "POST"])
def set_webhook():
    # we use the bot object to link the bot to our app which live
    # in the link provided by URL
    s = bot.setWebhook("{URL}{HOOK}".format(URL=URL, HOOK=TOKEN))
    # something to let us know things work
    if s:
        return "webhook setup ok"
    else:
        return "webhook setup failed"


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
    # logger.info(str(db.query_db("Select * from Humidity")))
    # If exceed threshold, send reminder (once every 30 mins)
    return "OK", 200


@app.route("/access", methods=["POST"])
def saveaccess():
    accesscode = request.values.get("id")
    """ TODO
    1. Check if id exists in database.
    2. If yes, then disable access control (marked as var in db) for 30 mins.
    3. Anyway, send relevant command to queue.
    """
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
    """
    Retrieve command from db.
    Craft suitable json response.
    """
    return jsonify({"a": "b"})


@app.route("/")
def index():
    return jsonify({"message": "Hello,hello, World!"})


# @app.route("/db")
# def whh():
#     cur = db.get_db().cursor()
#     results = []
#     for user in db.query_db("select * from user"):
#         print(user["id"], user["username"])
#     return str(results)


# @app.route("/dbadd")
# def wh67h():
#     conn = db.get_db()
#     cur = conn.cursor()
#     cur.execute("INSERT INTO user VALUES (3,'abc','234')")
#     conn.commit()
#     conn.close()
#     return "Success!"


if __name__ == "__main__":
    db.del_dball("Humidity")
    app.run(host='0.0.0.0',port=9876,threaded=True)
