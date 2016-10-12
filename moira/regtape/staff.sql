set head off
set feedback off
set pagesize 0
set linesize 512
spool /moira/load/staff.input.unsorted
select mit_id || rpad(nvl(last_name, ' '), 30, ' ') || rpad(nvl(first_name, ' '), 30, ' ') || rpad(nvl(middle_name, ' '), 30, ' ') || rpad(substr(nvl(office_location, ' '), 0, 12), 12, ' ') || rpad(substr(nvl(secondary_office_location, ' '), 0, 12), 12, ' ') || lpad(substr(nvl(office_phone, ' '), 0, 12), 12, ' ') || lpad(substr(nvl(secondary_office_phone, ' '), 0, 12), 12, ' ') || rpad(substr(nvl(department_name, ' '), 0, 50), 50, ' ') || rpad(substr(nvl(directory_title, ' '), 0, 50), 50, ' ') || rpad(substr(nvl(employee_type, ' '), 0, 40), 40, ' ') || rpad(substr(nvl(lower(email_address), ' '), 0, 100), 100, ' ') || rpad(substr(nvl(department_number, '0'), 0, 6), 6, ' ') from wareuser.moira_employee;
spool off
quit
