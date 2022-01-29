import sqlite3 as sql
import hashlib
import os


def hash_my_password(password, salt):
    scrypt_key = hashlib.scrypt(password=password.encode(), salt=salt, n=16384, r=8, p=1)
    return scrypt_key


def add_user(conn: sql.Connection, email, username, password):
    cur = conn.cursor()
    salt = os.urandom(16)
    hash = hash_my_password(password, salt)
    user_data = (email, username, hash, salt, None, 0)
    cur.execute("INSERT INTO userdata VALUES (?, ?, ?, ?, ?, ?)", user_data)
    conn.commit()
    return True


def authenticate_user(conn: sql.Connection, user, password):
    cur = conn.cursor()
    cur.execute('SELECT salt, hash, username FROM userdata WHERE username=? OR email=?', (user, user,))
    db_data = cur.fetchone()
    if db_data is None:
        return False, None

    salt = db_data[0]
    good_hash = db_data[1]
    user_name = db_data[2]

    auth_hash = hash_my_password(password, salt)
    if good_hash == auth_hash:
        return True, user_name
    else:
        return False, None