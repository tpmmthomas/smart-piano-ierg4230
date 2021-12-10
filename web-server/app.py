from flask import Flask, request, jsonify
import telegram
from telebot.credentials import bot_token, URL
from telebot.mastermind import get_response
import db

global bot
global TOKEN
TOKEN = bot_token
bot = telegram.Bot(token=TOKEN)
app = Flask(__name__)


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


@app.route("/")
def index():
    return jsonify({"message": "Hello,hello, World!"})


@app.route("/db")
def whh():
    cur = db.get_db().cursor()
    results = []
    for user in db.query_db("select * from user"):
        print(user["id"], user["username"])
    return str(results)


@app.route("/dbadd")
def wh67h():
    conn = db.get_db()
    cur = conn.cursor()
    cur.execute("INSERT INTO user VALUES (3,'abc','234')")
    conn.commit()
    conn.close()
    return "Success!"


if __name__ == "__main__":
    db.init_db(app)
    app.run(threaded=True)
