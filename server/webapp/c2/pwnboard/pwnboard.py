import requests

"""
integration
"""
def send_update(ip, host, name="nginxworker"):
    host = "http://" + host + "/generic"
    data = {'ip': ip, 'type': name}
    try:
        req = requests.post(host, json=data, timeout=3)
        print(req.text)
        return True
    except Exception as E:
        print(E)
        return False