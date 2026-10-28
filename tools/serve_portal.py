#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Serve the local Pages tree under its real /nauclerus/ prefix; no hardware access."""
import argparse
from http.server import SimpleHTTPRequestHandler,ThreadingHTTPServer
from pathlib import Path
class Handler(SimpleHTTPRequestHandler):
    def __init__(self,*a,**kw):super().__init__(*a,directory=str(Path(__file__).resolve().parents[1]/'build/pages'),**kw)
    def do_GET(self):
        if self.path=='/':self.send_response(302);self.send_header('Location','/nauclerus/');self.end_headers();return
        if not self.path.startswith('/nauclerus/'):
            self.send_error(404);return
        self.path=self.path[len('/nauclerus'):];super().do_GET()
    def end_headers(self):
        self.send_header('Cache-Control','no-store');super().end_headers()
if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--port',type=int,default=8765);a=p.parse_args()
    print(f'Local preview: http://127.0.0.1:{a.port}/nauclerus/',flush=True)
    ThreadingHTTPServer(('127.0.0.1',a.port),Handler).serve_forever()
