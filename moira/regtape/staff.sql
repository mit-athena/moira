set head off
set feedback off
set pagesize 0
set linesize 271
spool /moira/load/staff.input.unsorted
select mit_id || rpad(nvl(full_name, ' '), 30, ' ') || rpad(substr(nvl(office_location, ' '), 0, 12), 12, ' ') || rpad(substr(nvl(secondary_office_location, ' '), 0, 12), 12, ' ') || lpad(substr(nvl(office_phone, ' '), 0, 12), 12, ' ') || lpad(substr(nvl(secondary_office_phone, ' '), 0, 12), 12, ' ') || rpad(substr(department_name, 0, 50), 50, ' ') || substr(directory_title, 0, 50) from wareuser.moira_employee2;
spool off
quit
