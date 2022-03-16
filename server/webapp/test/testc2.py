import unittest
from urllib import response
import requests


# Required to decode data from C2
# Couldn't figure out how to import from c2.util properly so for now, the code is just copied over
def mal_decode(enc, key="CSEC476"):
    dec = ""
    for i in range(0, len(enc)):
        key_c = key[i % len(key)]
        placeholder = 127 + (enc[i]) - ord(key_c)
        dec_c = (placeholder % 127)
        dec += chr(dec_c)
    return dec


# Basic tests
class TestBase(unittest.TestCase):
    scheme="http"
    host="127.0.0.1"
    port=5000


    def setUp(self):
        self.url = f"{self.scheme}://{self.host}:{self.port}"


    # Tests that the home page can be reached
    def test_index_page_unauthenticated(self):
        response = requests.get(self.url + "/", allow_redirects=False)
        self.assertTrue(response.ok)
        self.assertTrue(response.is_redirect)  
        self.assertTrue(response.next.url.endswith("/login"))


# Authentication/session related tests
class TestAsUser(unittest.TestCase):
    session = requests.Session()
    scheme="http"
    host="127.0.0.1"
    port=5000
    user="admin"
    password="admin"
    email="admin@localhost"


    def setUp(self):
        self.url = f"{self.scheme}://{self.host}:{self.port}"    

    # Tests the user signup functionality 
    def test_1_signup(self):
        response = self.session.post(self.url + "/signup", data= {"email":self.email, "username":self.user, "password":self.password} )
        self.assertTrue(response.ok)


    # Tests that the user can successfully authenticate with the previously-created account
    # Will likely fail if test_signup fails
    def test_2_login(self):
        response = self.session.post(self.url + "/login", data= {"username":self.user, "password":self.password} )
        self.assertTrue(response.ok)
        self.assertTrue("session" in self.session.cookies)


    # Tests that an unauthenticated user cannot reach a page that requires authentication
    def test_3_stat_bots_unauthenticated(self):
        response = requests.get(self.url + "/api/v1/bots")
        self.assertFalse(response.ok)
        self.assertEqual(response.status_code, 401)


    # Tests that the authenticated user can reach a page that requires authentication
    def test_4_stat_bots_authenticated(self):
        response = self.session.get(self.url + "/api/v1/bots")
        self.assertTrue(response.ok)
        self.assertGreater(len(response.text), 0)


class TestAsBot(unittest.TestCase):
    session = requests.Session()
    scheme="http"
    host="127.0.0.1"
    port=5000


    def setUp(self):
        self.url = f"{self.scheme}://{self.host}:{self.port}"


    def test_heartbeat_1(self):
        response = self.session.post(self.url + "/heartbeat")
        self.assertTrue(response.ok)
        self.assertTrue("X-Session-ID" in self.session.cookies)
        print("UUID:" + self.session.cookies["X-Session-ID"])
        decoded = mal_decode(response.text.encode()) 
        self.assertEqual(decoded, '{"action": "info"}')


    def test_heartbeat_2(self):
        response = self.session.post(self.url + "/heartbeat")
        self.assertTrue(response.ok)
        decoded = mal_decode(response.text.encode()) 
        self.assertEqual(decoded, '{"action": "ok"}')
         

if __name__ == "__main__":
    TestBase.host = "192.168.1.14"
    TestAsBot.host = "192.168.1.14"
    TestAsUser.host = "192.168.1.14"
    unittest.TestLoader.sortTestMethodsUsing = None
    unittest.main(verbosity=2)