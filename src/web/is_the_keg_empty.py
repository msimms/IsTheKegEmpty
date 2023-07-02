#! /usr/bin/env python

import argparse
import flask
import json
import logging
import mako
import os
import sqlite3
import sys

from mako.template import Template

g_app = None
g_flask_app = flask.Flask(__name__)

HTML_DIR = 'html'

PARAM_KEG_ID = 'keg_id'
PARAM_READING = 'reading'
PARAM_READING_TIME = 'reading_time'

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
        sql = "create table status (id integer primary key, keg_id text, reading double, reading_time unsigned big int)"
        self.execute(sql)

    def create_reading(self, keg_id, reading, reading_time):
        return False

    def read_readings(self, keg_id):
        return []

class App(object):
    """Web app logic is stored here to keep it compartmentalized from the framework logic."""

    def __init__(self, root_url, root_dir):
        self.database = KegDatabase(root_dir, "keg.db")
        self.database.create_tables()
        self.root_url = root_url
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
        return False, ""

    def handle_api_create_login(self, values):
        return False, ""

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
