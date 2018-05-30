#!/moira/bin/python

import base64
import cx_Oracle as dbi
import fileinput
import os
import smtplib
import sys
import time
import urllib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from Crypto import Random
from Crypto.Cipher import AES

# configuration
hostname = os.uname()[1].lower()
subject= 'Register for your MIT Kerberos Account, %s %s'

TOKEN_EXPIRY = 604800
db_username = 'moira'
db_pwd = 'moira'

if hostname == "this-too-shall-pass.mit.edu":
    print "Running in TEST mode on %s" % (hostname)
    url = 'https://register-dev.mit.edu/reg/submittoken?token='
    fromaddr = 'accounts@mit.edu'
    replyto = 'accounts@mit.edu'
    subject = 'TEST: ' + subject
else:
    url = 'https://accounts.mit.edu/register/submittoken?token='
    fromaddr = 'accounts@mit.edu'
    replyto = 'accounts@mit.edu'

moira_base = '/moira/reg/'
keyfile = moira_base + 'reg.key'
text_template = moira_base + 'reg.txt'
html_template = moira_base + 'reg.html'

# crypto class
BS = 16
pad = lambda s: s + (BS - len(s) % BS) * chr(BS - len(s) % BS)
unpad = lambda s : s[0:-ord(s[-1])]

class AESCipher:

    def __init__( self, key ):
        self.key = key
        
    def encrypt( self, raw ):
        raw = pad(raw)
        iv = Random.new().read( AES.block_size )
        cipher = AES.new( self.key, AES.MODE_CBC, iv )
        return base64.b64encode( iv + cipher.encrypt( raw ) )
        
    def decrypt( self, enc ):
        enc = base64.b64decode(enc)
        iv = enc[:16]
        cipher = AES.new(self.key, AES.MODE_CBC, iv )
        return unpad(cipher.decrypt( enc[16:] ))

# main program
with open(keyfile, 'r') as f:
    reg_key = f.readline().rstrip("\r\n")
        
cipher = AESCipher(reg_key)

dbh = dbi.connect(db_username, db_pwd)
sth = dbh.cursor()

# Input is MITID, First, Last, external email, status
for line in fileinput.input():
    [ mitid, first, last, email, status ] = [ x.strip() for x in line.split(',')]
    now = int(time.time())

    # If we've ever issued a status 1 piece of mail, we're done.
    sth.execute('SELECT COUNT(*) FROM regmail WHERE status = 1 AND ((mitid = :mitid AND email = :email) OR (mitid = :mitid AND email = CHR(0)))',
                mitid=mitid, email=email)
    count = sth.fetchall()[0][0]

    if count > 0:
        print "Issued token for active user %s %s, MIT ID %s, email %s previously, skipping." % (first, last, mitid, email)
        continue

    # Select latest issuance time.
    sth.execute('SELECT MAX(issued) FROM regmail WHERE ((mitid = :mitid AND email = :email) OR (mitid = :mitid AND email = CHR(0)))', mitid=mitid, email=email)
    last_issued = sth.fetchall()[0][0]

    # no results, default to 0.
    if last_issued == None:
        last_issued = 0

    # Did we issue one of these in the last week?
    if now - last_issued < TOKEN_EXPIRY:
        print "Issued token for user %s %s, MIT ID %s, email %s too recently, skipping." % (first, last, mitid, email)
        continue

    token = '%s|%s|%s|%d' % (first, last, mitid, now)
    enc_token = cipher.encrypt(token)
    signup_url = url + urllib.quote_plus(enc_token)

    msg = MIMEMultipart('alternative')
    msg['Subject'] = subject % (first, last)
    msg['From'] = fromaddr
    msg['To'] = email
    msg['Reply-To'] = replyto

    with open(text_template, 'r') as f:
        text = f.read() % (first, last, signup_url)

    with open(html_template, 'r') as f:
        html = f.read() % (first, last, signup_url)

    part1 = MIMEText(text, 'plain')
    part2 = MIMEText(html, 'html')
    msg.attach(part1)
    msg.attach(part2)

    s = smtplib.SMTP('localhost')
    print "Sending link %s to user %s" % (signup_url, email)
    s.sendmail(fromaddr, email, msg.as_string())
    s.quit()

    # Update state table.
    sth.execute('INSERT INTO regmail(mitid, email, status, issued) VALUES (:mitid, :email, :status, :now)', mitid=mitid, email=email, status=status, now=now)
    dbh.commit()

sth.close()
dbh.close()
sys.exit(0)
