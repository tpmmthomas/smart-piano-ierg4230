import telegram
from telegram import ReplyKeyboardMarkup, ReplyKeyboardRemove, Update
from telegram.ext import (
    Updater,
    CommandHandler,
    MessageHandler,
    Filters,
    ConversationHandler,
    CallbackContext,
)
from telebot.credentials import bot_token, URL
import db

updater = Updater(bot_token)

def x(update: Update, context: CallbackContext):
    chat_id = update.message.chat.id
    name = update.message.from_user.first_name
    y = db.query_db(f'Select * from Usertb where "chat_id" = {chat_id}')
    if len(y) > 0:
        update.message.reply_text("You have already registered.")
        return
    db.add_db("Usertb",{"chat_id":chat_id,"name":name})
    update.message.reply_text(f"Welcome {name}! have successfully registered. You can now register new cards!")
    return

def reg(update: Update, context: CallbackContext): 
    chat_id = update.message.chat.id
    y = db.query_db(f'Select "name" from Usertb where "chat_id" = {chat_id}')
    try:
        code = context.args[0]
        update.message.reply_text(f"Your code is {code}.")
    except:
        update.message.reply_text("You need to provide the code.")
        return
    if len(y) == 0:
        update.message.reply_text("You are not a registered user.")
        return
    name = update.message.from_user.first_name
    db.add_db("RegisteredAccess",{'name':name,'code':code})
    update.message.reply_text("Code added successfully!")

def tune(update: Update, context: CallbackContext):
    chat_id = update.message.chat.id
    y = db.query_db(f'Select "name" from Usertb where "chat_id" = {chat_id}')
    if len(y) == 0:
        update.message.reply_text("You are not a registered user.")
        return
    db.add_db("Tuning",{"dummy":1})
    db.del_dball("TuneReminders")
    update.message.reply_text("Tuning record added!")


dispatcher = updater.dispatcher
dispatcher.add_handler(CommandHandler("start", x))
dispatcher.add_handler(CommandHandler("register", reg))
dispatcher.add_handler(CommandHandler("addtuning", tune))
updater.start_polling()
global bot 
bot = updater.bot
# updater.idle()