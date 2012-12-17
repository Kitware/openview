from os import curdir, sep
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer

class MyHandler(BaseHTTPRequestHandler):
    def do_GET(self):
        try:
            f = open(curdir + sep + self.path) #self.path has /test.html
            self.send_response(200)
            if self.path.endswith(".html"):
                self.send_header('Content-type', 'text/html')
            if self.path.endswith(".js"):
                self.send_header('Content-type', 'text/javascript')
            self.send_header("Cache-Control", "no-cache,no-store,must-revalidate")
            self.send_header("Pragma", "no-cache")
            self.send_header("Expires", 0)
            self.end_headers()
            self.wfile.write(f.read())
            f.close()
            return
        except IOError:
            self.send_error(404,'File Not Found: %s' % self.path)
     
def main():
    try:
        server = HTTPServer(('', 8888), MyHandler)
        print 'started httpserver on 8888...'
        server.serve_forever()
    except KeyboardInterrupt:
        print '^C received, shutting down server'
        server.socket.close()

if __name__ == '__main__':
    main()

