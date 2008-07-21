set head off
set feedback off
set pagesize 0
set linesize 256
spool /moira/load/student.input.unsorted
select rpad(nvl(mit_id, ' '), 9, ' ') || rpad(nvl(last_name, ' '), 30, ' ') || rpad(nvl(first_name, ' '), 30, ' ') || rpad(nvl(middle_name, ' '), 30, ' ') || rpad(nvl(student_year, ' '), 1, ' ') || rpad(nvl(department, ' '), 4, ' ') || rpad(nvl(term_street1, ' '), 30, ' ') || rpad(nvl(term_city, ' '), 40, ' ') || rpad(nvl(term_state, ' '), 2, ' ') || rpad(nvl(term_zip, ' '), 9, ' ') || rpad(nvl(term_phone1, ' '), 20, ' ') || rpad(nvl(office_location, ' '), 30, ' ') || rpad(nvl(office_phone, ' '), 20, ' ') from wareuser.moira_student4;
spool off
quit
