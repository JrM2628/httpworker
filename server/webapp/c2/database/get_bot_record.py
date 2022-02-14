import json
import sqlite3 as sql
from datetime import datetime


def get_bot_info(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT * FROM bots WHERE uuid=?''', (id,))
    return cur.fetchone()


def server_stats_to_dict(conn:sql.Connection):
    cur = conn.cursor()
    server_stats = {}
    cur.execute('SELECT COUNT(uuid) FROM bots')
    server_stats["bots"] = cur.fetchone()[0]
    cur.execute('SELECT COUNT(username) FROM userdata')
    server_stats["users"] = cur.fetchone()[0]
    return server_stats


def bots_to_dict(conn:sql.Connection):
    cur = conn.cursor()
    cur.execute('''SELECT uuid, checkin, networkaddresses FROM bots ORDER BY checkin DESC''')
    data = cur.fetchall()
    bots = {}
    if data is None:
        return bots
        
    for bot in data:
        uuid = bot[0]
        checkin = bot[1]
        nwaddresses = bot[2]
        ips = []
        if nwaddresses != "":
            nwaddresses = json.loads(nwaddresses)
            for adapter in nwaddresses:
                if "ip" in nwaddresses[adapter]:
                    ips.append(nwaddresses[adapter]["ip"])
        bots[uuid] = {"checkin": checkin, "ips": ips}
    return bots


def bot_ips(conn:sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT networkaddresses FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    if data is None:
        return []
    nwaddresses = data[0]
    ips = []
    if nwaddresses != "":
        nwaddresses = json.loads(nwaddresses)
        for adapter in nwaddresses:
            if "ip" in nwaddresses[adapter]:
                ips.append(nwaddresses[adapter]["ip"])
    return ips


def bot_to_dict(conn:sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT uuid, checkin, ip, username, devicename, region, memory, commandqueue, os FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    bot = {}
    if data is None:
        return bot
    bot["uuid"] = data[0]
    bot["checkin"] = data[1]
    bot["public_ip"] = data[2]
    if(data[8]):
        bot["os"] = json.loads(data[8])
    else:
        bot["os"] = {}
    bot["username"] = data[3]
    bot["devicename"] = data[4]
    bot["region"] = data[5]
    bot["memory"] = data[6]
    bot["queue"] = data[7]
    return bot


def process_list_to_dict(conn:sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT processlist FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    print(data)
    process_list = {}
    if data is None or data[0] is None:
        return process_list
    process_list = json.loads(data[0])
    return process_list


def network_info_to_dict(conn:sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT networkaddresses FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    network_info = {}
    if data is None or data[0] is None:
        return network_info
    
    json_data = data[0]
    return json_data


def command_history_to_dict(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute("SELECT time, command, output FROM output WHERE uuid=? ORDER BY time DESC", (id,))
    data = cur.fetchall()
    command_history = {}
    if data is None:
        return command_history
    
    for bot in data:
        time = bot[0]
        command = bot[1]
        output = bot[2]
        command_history[time] = {"command":command, "output":output} 
    return command_history


"""
    =====================================================================================
        Everything below this line is legacy and should be phased out
        Everything above the line returns dictionaries which are used to return JSON strings
"""
def get_bot_commandqueue(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute("SELECT commandqueue FROM bots WHERE uuid=?", (id,))
    return cur.fetchone()


def get_bot_count(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT COUNT(uuid) FROM bots')
    return_val = cur.fetchone()[0]
    return return_val


def get_user_count(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT COUNT(username) FROM userdata')
    return_val = cur.fetchone()[0]
    return return_val