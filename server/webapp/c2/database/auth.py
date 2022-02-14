import sqlite3 as sql
import hashlib
import os


def hash_my_password(password, salt):
    """Returns bytes of a password hashed in scrypt

    Args:
        password (str): String of password
        salt (bytes): Randomly-generated salt

    Returns:
        bytes: Scrypt key
    """    
    scrypt_key = hashlib.scrypt(password=password.encode(), salt=salt, n=16384, r=8, p=1)
    return scrypt_key


def add_user(conn: sql.Connection, email, username, password):
    """Adds a user to the user table

    Args:
        conn (sql.Connection): Connection to database
        email (str): Email address of user
        username (str): Username
        password (str): Password

    Returns:
        bool: true if successful
    """    
    if email == "" or username == "" or password == "":
        return False
    cur = conn.cursor()
    salt = os.urandom(16)
    hash = hash_my_password(password, salt)
    user_data = (email, username, hash, salt, None, 0)
    cur.execute("INSERT INTO userdata VALUES (?, ?, ?, ?, ?, ?)", user_data)
    conn.commit()
    return True


def authenticate_user(conn: sql.Connection, user, password):
    """Authenticates a user with given credentials

    Args:
        conn (sql.Connection): Connection to database
        user (str): Username or email address
        password (str): Password

    Returns:
        bool: true if authenticated successfully (hashes match), false otherwise
    """    
    cur = conn.cursor()
    cur.execute('SELECT salt, hash, username FROM userdata WHERE username=? OR email=?', (user, user,))
    db_data = cur.fetchone()
    if db_data is None:
        return False, None

    salt = db_data[0]
    stored_hash = db_data[1]
    user_name = db_data[2]

    login_hash = hash_my_password(password, salt)
    if stored_hash == login_hash:
        return True, user_name
    else:
        return False, None