import db

def get_response(msg, chat_id,name):
    cmdlist = msg.split(" ")
    cmd = cmdlist[0]
    if cmd == "/start":
        y = db.query_db('Select * from User where "chat_id" = \'{chat_id}\'')
        if len(y) > 0:
            return "You have already registered."
        db.add_db("User",{"chat_id":chat_id,"name":name})
        return f"Welcome {name}! have successfully registered. You can now register new cards!"
    elif cmd == "/register":
        y = db.query_db('Select name from User where "chat_id" = \'{chat_id}\'')
        if len(y) == 0:
            return "You are not a registered user."
        if len(cmd) < 2:
            return "You need to provide the code."
        name = y[0]['name']
        db.add_db("RegisteredAccess",{'name':name,'code':cmd[1]})
        return "Code added successfully!"
    elif cmd == "/addtuning":
        db.add_db("Tuning",{"dummy":1})
    return "0.0"
