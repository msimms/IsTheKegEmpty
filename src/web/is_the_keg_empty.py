#! /usr/bin/env python

import argparse
import bcrypt
import flask
import json
import logging
import mako
import os
import sqlite3
import sys
import InputChecker

from urllib.parse import unquote_plus
from mako.template import Template

g_app = None
g_flask_app = flask.Flask(__name__)

MIN_PASSWORD_LEN  = 8
HTML_DIR = 'html'
PARAM_KEG_ID = 'keg_id'
PARAM_READING = 'reading'
PARAM_READING_TIME = 'reading_time'
PARAM_USERNAME = "username" # Login name for a user
PARAM_REALNAME = "realname" # User's real name
PARAM_PASSWORD = "password" # User's password
PARAM_PASSWORD1 = "password1" # User's password when creating an account
PARAM_PASSWORD2 = "password2" # User's confirmation password when creating an account
PARAM_SESSION_TOKEN = "session_token"
PARAM_SESSION_EXPIRY = "session_expiry"

class ApiException(Exception):
    """Exception thrown by a REST API."""

    def __init__(self, *args):
        Exception.__init__(self, args)

    def __init__(self, code, message):
        self.code = code
        self.message = message
        Exception.__init__(self, code, message)

class ApiMalformedRequestException(ApiException):
    """Exception thrown by a REST API when an API request is missing required parameters."""

    def __init__(self, message):
        ApiException.__init__(self, 400, message)

class ApiAuthenticationException(ApiException):
    """Exception thrown by a REST API when user authentication fails."""

    def __init__(self, message):
        ApiException.__init__(self, 401, message)

class ApiNotLoggedInException(ApiException):
    """Exception thrown by a REST API when the user is not logged in."""

    def __init__(self):
        ApiException.__init__(self, 403, "Not logged in")

class Database(object):
    """Base class for a database. Encapsulates common functionality."""
    db_file = ""

    def __init__(self):
        super(Database, self).__init__()

    def log_error(self, log_str):
        """Writes an error message to the log file."""
        logger = logging.getLogger()
        logger.error(log_str)

    def is_quoted(self, log_str):
        """Determines if the provided string starts and ends with a double quote."""
        if len(log_str) < 2:
            return False
        return log_str[0] == '\"' and log_str[len(log_str)-1] == '\"'

    def quote_identifier(self, log_str, errors="strict"):
        """Adds quotes to the given string if they do not already exist."""
        if self.is_quoted(log_str):
            return log_str
        encodable = log_str.encode("utf-8", errors).decode("utf-8")
        null_index = encodable.find("\x00")
        if null_index >= 0:
            return ""
        return "\"" + encodable.replace("\"", "\"\"") + "\""

class SqliteDatabase(Database):
    """Abstract Sqlite database implementation."""

    def __init__(self, root_dir, file_name):
        self.db_file_name = os.path.join(root_dir, file_name)
        Database.__init__(self)

    def connect(self):
        """Inherited from the base class and unused."""
        pass

    def execute(self, sql):
        """Executes the specified SQL query."""
        try:
            con = sqlite3.connect(self.db_file_name)
            with con:
                cur = con.cursor()
                cur.execute(sql)
                return cur.fetchall()
        except:
            self.log_error("Database error:\n\tfile = " + self.db_file_name + "\n\tsql = " + self.quote_identifier(sql))
        finally:
            if con:
                con.close()
        return None

class KegDatabase(SqliteDatabase):
    """Database implementation."""

    def __init__(self, root_dir, file_name):
        SqliteDatabase.__init__(self, root_dir, file_name)

    def create_tables(self):
        sql = "create table user (id integer primary key, username text, realname text, passhash text)"
        self.execute(sql)
        sql = "create table status (id integer primary key, keg_id text, reading double, reading_time unsigned big int)"
        self.execute(sql)

    def create_user(email, realname, computed_hash):
        sql = "insert into user "
        return False

    def retrieve_user(self, email):
        return None

    def delete_user(self, email):
        return None

    def create_reading(self, keg_id, reading, reading_time):
        sql = "insert into status "
        return False

    def retrieve_readings(self, keg_id):
        return []

    def delete_readings(self, keg_id):
        return None

class UserMgr(object):
    """Encapsulates user authentication and management."""

    def __init__(self, database):
        self.database = database
        super(Database, self).__init__()

    def authenticate_user(self, email, password):
        """Validates a user against the credentials in the database."""
        if self.database is None:
            raise Exception("No database.")
        if len(email) == 0:
            raise Exception("An email address not provided.")
        if len(password) < MIN_PASSWORD_LEN:
            raise Exception("The password is too short.")

        _, db_hash1, _ = self.database.retrieve_user(email)
        if db_hash1 is None:
            raise Exception("The user (" + email + ") could not be found.")

        if isinstance(password, str):
            password = password.encode()
        if isinstance(db_hash1, str):
            db_hash1 = db_hash1.encode()
        return bcrypt.checkpw(password, db_hash1)

    def create_user(self, email, realname, password1, password2):
        """Adds a user to the database."""
        if self.database is None:
            raise Exception("No database.")
        if len(email) == 0:
            raise Exception("An email address not provided.")
        if len(realname) == 0:
            raise Exception("Name not provided.")
        if len(password1) < MIN_PASSWORD_LEN:
            raise Exception("The password is too short.")
        if password1 != password2:
            raise Exception("The passwords do not match.")
        if self.database.retrieve_user(email) is None:
            raise Exception("The user already exists.")

        salt = bcrypt.gensalt()
        computed_hash = bcrypt.hashpw(password1.encode('utf-8'), salt)
        if not self.database.create_user(email, realname, computed_hash):
            raise Exception("An internal error was encountered when creating the user.")

        return True

class App(object):
    """Web app logic is stored here to keep it compartmentalized from the framework logic."""

    def __init__(self, root_url, root_dir):
        self.database = KegDatabase(root_dir, "keg.db")
        self.database.create_tables()
        self.root_url = root_url
        self.user_mgr = UserMgr(self.database)
        super(App, self).__init__()

    def index(self):
        """Renders the index page."""
        try:
            html_file = os.path.join(self.root_dir, HTML_DIR, 'index.html')
            my_template = Template(filename=html_file, module_directory=self.tempmod_dir)
            return my_template.render(root_url=self.root_url)
        except:
            pass
        return ""

    def handle_api_login(self, values):
        if self.user_id is not None:
            return True, ""

        # Required parameters.
        if PARAM_USERNAME not in values:
            raise ApiException.ApiAuthenticationException("Username not specified.")
        if PARAM_PASSWORD not in values:
            raise ApiException.ApiAuthenticationException("Password not specified.")

        # Decode and validate the required parameters.
        email = unquote_plus(values[PARAM_USERNAME])
        if not InputChecker.is_email_address(email):
            raise ApiException.ApiAuthenticationException("Invalid email address.")
        password = unquote_plus(values[PARAM_PASSWORD])

        try:
            if not self.user_mgr.authenticate_user(email, password):
                raise ApiException.ApiAuthenticationException("Authentication failed.")
        except Exception as e:
            raise ApiException.ApiAuthenticationException(str(e))

        cookie, expiry = self.user_mgr.create_new_session(email)
        if not cookie:
            raise ApiException.ApiAuthenticationException("Session cookie not generated.")
        if not expiry:
            raise ApiException.ApiAuthenticationException("Session expiry not generated.")

        session_data = {}
        session_data[PARAM_SESSION_TOKEN] = cookie
        session_data[PARAM_SESSION_EXPIRY] = expiry
        json_result = json.dumps(session_data, ensure_ascii=False)

        return True, json_result

    def handle_api_create_login(self, values):
        if self.user_id is not None:
            raise Exception("Already logged in.")

        # Required parameters.
        if PARAM_USERNAME not in values:
            raise ApiException.ApiAuthenticationException("Username not specified.")
        if PARAM_REALNAME not in values:
            raise ApiException.ApiAuthenticationException("Real name not specified.")
        if PARAM_PASSWORD1 not in values:
            raise ApiException.ApiAuthenticationException("Password not specified.")
        if PARAM_PASSWORD2 not in values:
            raise ApiException.ApiAuthenticationException("Password confirmation not specified.")

        # Decode and validate the required parameters.
        email = unquote_plus(values[PARAM_USERNAME])
        if not InputChecker.is_email_address(email):
            raise ApiException.ApiMalformedRequestException("Invalid email address.")
        realname = unquote_plus(values[PARAM_REALNAME])
        if not InputChecker.is_valid_decoded_str(realname):
            raise ApiException.ApiMalformedRequestException("Invalid name.")
        password1 = unquote_plus(values[PARAM_PASSWORD1])
        password2 = unquote_plus(values[PARAM_PASSWORD2])

        try:
            if not self.user_mgr.create_user(email, realname, password1, password2):
                raise Exception("User creation failed.")
        except:
            raise Exception("User creation failed.")

        cookie, expiry = self.user_mgr.create_new_session(email)
        if not cookie:
            raise ApiException.ApiAuthenticationException("Session cookie not generated.")
        if not expiry:
            raise ApiException.ApiAuthenticationException("Session expiry not generated.")

        session_data = {}
        session_data[PARAM_SESSION_TOKEN] = cookie
        session_data[PARAM_SESSION_EXPIRY] = expiry
        json_result = json.dumps(session_data, ensure_ascii=False)

        return True, json_result

    def handle_api_login_status(self, values):
        return False, ""

    def handle_api_logout(self, values):
        return False, ""

    def handle_api_keg_status(self, values):
        keg_id = values[PARAM_KEG_ID]
        self.database.read_readings(keg_id)
        return False, ""

    def handle_api_register_key(self, values):
        return False, ""

    def handle_api_update_keg_weight(self, values):
        keg_id = values[PARAM_KEG_ID]
        reading = values[PARAM_READING]
        reading_time = values[PARAM_READING_TIME]
        self.database.create_reading(keg_id, reading, reading_time)
        return False, ""

    def handle_api_1_0_get_request(self, request, values):
        """Called to parse a version 1.0 API GET request."""
        if request == 'login':
            return self.handle_api_login(request, values)
        if request == 'login_status':
            return self.handle_api_login_status(request, values)
        if request == 'keg_status':
            return self.handle_api_keg_status(request, values)
        return False, ""

    def handle_api_1_0_post_request(self, request, values):
        """Called to parse a version 1.0 API POST request."""
        if request == 'create_login':
            return self.handle_api_create_login(request, values)
        if request == 'logout':
            return self.handle_api_logout(request, values)
        if request == 'register_keg':
            return self.handle_api_register_key(request, values)
        if request == 'update_keg_weight':
            return self.handle_api_update_keg_weight(request, values)
        return False, ""

    def handle_api_1_0_delete_request(self, request, values):
        """Called to parse a version 1.0 API DELETE request."""
        return False, ""

    def api(self, verb, request, values):
        """Handles API requests."""
        request = request.lower()
        if verb == 'GET':
            return self.handle_api_1_0_get_request(request, values)
        elif verb == 'POST':
            return self.handle_api_1_0_post_request(request, values)
        elif verb == 'DELETE':
            return self.handle_api_1_0_delete_request(request, values)
        return False, ""

@g_flask_app.route('/')
def index():
    """Renders the index page."""
    global g_app
    return g_app.index()

@g_flask_app.route('/api/<version>/<method>', methods = ['GET','POST','DELETE'])
def api(version, method):
    """Endpoint for API calls."""
    global g_app
    response = ""
    code = 200
    try:
        # The the API params.
        if flask.request.method == 'GET':
            verb = "GET"
            params = flask.request.args
        elif flask.request.method == 'DELETE':
            verb = "DELETE"
            params = flask.request.args
        elif flask.request.data:
            verb = "POST"
            params = json.loads(flask.request.data)
        else:
            verb = "GET"
            params = ""

        # Process the API request.
        if version == '1.0':
            handled, response = g_app.api(verb, method, params)
            if not handled:
                code = 400
        else:
            code = 400
    except:
        pass
    return response, code

def check():
    pass

def main():
    global g_app
    global g_flask_app

    # Parse command line options.
    parser = argparse.ArgumentParser()

    try:
        args = parser.parse_args()
    except IOError as e:
        parser.error(e)
        sys.exit(1)

    mako.collection_size = 100
    mako.directories = "templates"

    root_dir = os.path.dirname(os.path.abspath(__file__))
    g_app = App("", root_dir)
    g_flask_app.run()

if __name__=="__main__":
	main()
