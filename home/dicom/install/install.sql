DROP DATABASE IF EXISTS primal;
CREATE DATABASE IF NOT EXISTS primal;
use primal;
create table patient (puid varchar(24) not null primary key, 
                      pname varchar(38) not null, 
                      pid varchar(38) not null,
                      pdob date,
					  psex varchar(6),
					  pclientid varchar(16),
					  PatientComments varchar(128),
					  INDEX pname (pname));
create table receive (puid varchar(24) not null,
					  rservername varchar(64) not null,
                      tstartrec datetime not null,
                      tendrec datetime,
					  ttimestamp int(10) DEFAULT 0,
                      rec_images int(6) DEFAULT 0,
                      rerror int(1),
                      senderAET varchar(128),
					  callingAET varchar(32),
					  primary key (puid, rservername),
					  INDEX startrec (tstartrec));
create table process (puid varchar(24) not null,
					  pservername varchar(64) not null,
                      tstartproc datetime not null,
                      tendproc datetime,
                      perror int(1) DEFAULT 0,
					  primary key (puid, pservername));
create table send    (puid varchar(24) not null,
					  sservername varchar(64) not null,
					  tdestnum tinyint,
					  index (puid),
                      tdest varchar(38) not null,
                      tstartsend datetime not null,
                      tendsend datetime,
                      timages int(6),
                      serror int(1) not null default 0,
					  sretry int(1) not null default 0,
					  complete int(1));
create table user	 (loginid varchar(36) not null primary key,
					  login_sec_level int(5) not null,
					  username varchar(36) not null,
					  active int(1),
					  password varchar(70) not null,
					  page_size int(4),
					  refresh_delay int(4),
					  sec_bit varchar(128),
					  rec_sec varchar(4096),
					  access datetime);
create table study   (DicomCasesID int(10) UNIQUE not null AUTO_INCREMENT,
					  SIUID varchar(68) not null,
                      puid varchar(24) not null,
                      sServerName varchar(64) not null,
                      StudyDesc varchar(256),
					  StudyNumImg int(8),
					  AccessionNum varchar(32),
					  StudyDate datetime,
					  StudyModType varchar(16),
					  sClientID varchar(16),
					  sClientName varchar(256),
					  sCaseID varchar(16) DEFAULT "0",
					  sRequestedProcedureID varchar(66),
					  primary key (puid, SIUID));
create table series  (SIUID varchar(68) not null,
                      SERIUID varchar(68),
                      puid varchar(24) not null,
                      SeriesDesc varchar(256),
					  SeriesNumImg int(8),
					  Modality varchar(4),
					  primary key (puid, SERIUID));
create table image   (SOPIUID varchar(68) not null,
                      SERIUID varchar(68),
                      puid varchar(24) not null,
					  iservername varchar(64) not null,
					  ifilename varchar(128),
                      idate datetime,
                      iarchive varchar(128) not null default 'local',
                      ilocation varchar(256),
                      TransferSyntaxUID varchar(64),
                      primary key (SOPIUID, puid, iservername));
create table monitor (SCP varchar(4) not null primary key,
					  status tinyint(4) not null,
					  begin_state datetime not null,
					  ts TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP);
create table locker  (pproc varchar(128) not null primary key,
					  ptimestamp datetime,
					  puser varchar(128) not null default 'none',
					  plock int(2));
create table QR      (prefetch_results_id int(10) UNSIGNED UNIQUE not null AUTO_INCREMENT,
                      puid varchar(24) not null,
					  SIUID varchar(68) not null,
                      prefetch_pending_id int(10) UNSIGNED,
					  qraet varchar(32),
					  qraec varchar(32),
					  qrport smallint(5) UNSIGNED,
					  qrip varchar(32),
					  qrstatus varchar(16) not null default 'Hold',
                      pname varchar(64),
					  pid varchar(32),
					  psex varchar(8),
					  pbday datetime,
					  study_date date,
					  study_time time,
					  Accession varchar(32),
					  study_description varchar(255),
					  body_part_examined varchar(255),
					  modality varchar(8),
					  QueryDate datetime,
					  RetrieveDate datetime,
					  NumImages int(8),
					  primary key (puid, SIUID));
create table Prefetch (prefetch_id int(10) UNSIGNED UNIQUE not null AUTO_INCREMENT,
                      puid varchar(24) not null,
					  qraet varchar(32),
					  qraec varchar(32),
					  qrport smallint(5) UNSIGNED,
					  qrip varchar(32));
create table conf_rec (conf_rec_id bigint(20) UNSIGNED UNIQUE not null AUTO_INCREMENT,
				      conf_name varchar(64) not null default 'default',
					  conf_server varchar(128) not null default 'localhost',
					  rec_type varchar(32) not null default 'dicom',
					  rec_port int(5) default 2000,
					  rec_dir varchar(128) not null default '/home/dicom/receive',
					  rec_log_full_path varchar(128) not null default '/var/log/primal/receive.log',
					  rec_log_level varchar(32) not null default 'debug',
					  rec_aet varchar(32) not null default 'PRIMAL',
					  rec_time_out int(5) not null default 10,
					  proc_dir varchar(128) not null default '/home/dicom/process',
					  proc_log_full_path varchar(128) not null default '/var/log/primal/process.log',
					  out_dir varchar(128) not null default '/home/dicom/send',
					  out_log_full_path varchar(128) not null default '/var/log/primal/send.log',
					  sent_dir varchar(128) not null default '/home/dicom/sent',
					  hold_dir varchar(128) not null default '/home/dicom/hold',
					  error_dir varchar(128) not null default '/home/dicom/error',
					  dupe int(5) not null default 0,
					  pass_through int(1) not null default 0,
					  ret_period int(10) not null default 4320,
				      active int(1) not null default 1,
 				      primary key (conf_rec_id));
create table conf_proc (conf_proc_id int(10) UNSIGNED UNIQUE not null AUTO_INCREMENT,
				      conf_rec_id bigint(20) UNSIGNED not null default 0,
				      proc_name varchar(64) not null default 'default',
					  proc_type varchar(32) not null default 'tag',
					  proc_perator varchar(32) not null default '=',
					  proc_cond varchar(32) not null default '0',
					  proc_action varchar(8) not null default 'allow',
					  proc_order int(5) not null default 0, 
					  proc_dest int(10) default 0,
				      active int(1) not null default 1,
				      primary key (conf_proc_id));
create table conf_send (conf_send_id int(10) UNSIGNED UNIQUE not null AUTO_INCREMENT,
				      conf_rec_id bigint(20) UNSIGNED not null default 0,
				      send_name varchar(64) not null default 'default',
					  send_aet varchar(32) not null default 'PRIMAL',
  					  send_aec varchar(32) not null default 'PRIMAL',
					  send_hip varchar(32) not null default '0.0.0.0',
					  send_type varchar(32) not null default 'dicom',
					  send_port int(5) default 2000,
					  send_time_out int(5) not null default 10,
					  send_comp_level int(5) not null default 0,
					  send_retry int(5) not null default 3,
					  send_username varchar(32) not null default 'primal',
					  send_password varchar(32) not null default 'primal',
				      active int(1) not null default 1,
				      primary key (conf_send_id));
create table page_columns (id int(4) not null primary key,
					  name varchar(32) not null,
					  dorder int(4) not null);
create table user_columns	(user_id varchar(32) not null primary key,
							Column1_Visible BOOLEAN not null default FALSE,
							Column1_Order tinyint(4) not null default 1,
							Column2_Visible BOOLEAN not null default FALSE,
							Column2_Order tinyint(4) not null default 2,
							Column3_Visible BOOLEAN not null default FALSE,
							Column3_Order tinyint(4) not null default 3,
							Column4_Visible BOOLEAN not null default FALSE,
							Column4_Order tinyint(4) not null default 4,
							Column5_Visible BOOLEAN not null default FALSE,
							Column5_Order tinyint(4) not null default 5,
							Column6_Visible BOOLEAN not null default FALSE,
							Column6_Order tinyint(4) not null default 6,
							Column7_Visible BOOLEAN not null default FALSE,
							Column7_Order tinyint(4) not null default 7,
							Column8_Visible BOOLEAN not null default FALSE,
							Column8_Order tinyint(4) not null default 8,
							Column9_Visible BOOLEAN not null default FALSE,
							Column9_Order tinyint(4) not null default 9,
							Column10_Visible BOOLEAN not null default FALSE,
							Column10_Order tinyint(4) not null default 10,
							Column11_Visible BOOLEAN not null default FALSE,
							Column11_Order tinyint(4) not null default 11,
							Column12_Visible BOOLEAN not null default FALSE,
							Column12_Order tinyint(4) not null default 12,
							Column13_Visible BOOLEAN not null default FALSE,
							Column13_Order tinyint(4) not null default 13,
							Column14_Visible BOOLEAN not null default FALSE,
							Column14_Order tinyint(4) not null default 14,
							Column15_Visible BOOLEAN not null default FALSE,
							Column15_Order tinyint(4) not null default 15,
							Column16_Visible BOOLEAN not null default FALSE,
							Column16_Order tinyint(4) not null default 16,
							Column17_Visible BOOLEAN not null default FALSE,
							Column17_Order tinyint(4) not null default 17,
							Column18_Visible BOOLEAN not null default FALSE,
							Column18_Order tinyint(4) not null default 18);
create table ticket			(ticket_num INT(8) not null AUTO_INCREMENT PRIMARY key, 
							destination varchar(64) not null, 
							proc_id INT(8) not null);
create table procedure_matching	(procedure_id int(10) UNSIGNED UNIQUE not null AUTO_INCREMENT, 
	modality varchar (8) not null,
	study_description varchar(256),
	body_part varchar(64)
	);
grant all privileges on primal.* to 'primal'@'localhost' identified by 'primal' with grant option;
insert into user (loginid, login_sec_level, username, active, password, rec_sec) values('primal', '255', 'primal', '1', 'cHJpbWFs', 'ALL');
insert into page_columns (id, name, dorder) values(1, 'Start Receive', 1);
insert into page_columns (id, name, dorder) values(2, 'Patient Name', 2);
insert into page_columns (id, name, dorder) values(3, 'Patient ID', 3);
insert into page_columns (id, name, dorder) values(4, 'Accession #', 4);
insert into page_columns (id, name, dorder) values(5, 'DOB', 5);
insert into page_columns (id, name, dorder) values(6, 'MOD', 6);
insert into page_columns (id, name, dorder) values(7, 'Study Date', 7);
insert into page_columns (id, name, dorder) values(8, 'End Receive', 8);
insert into page_columns (id, name, dorder) values(9, '# images rec', 9);
insert into page_columns (id, name, dorder) values(10, 'Start Process Date', 10);
insert into page_columns (id, name, dorder) values(11, 'End Process Date', 11);
insert into page_columns (id, name, dorder) values(12, 'Destination', 12);
insert into page_columns (id, name, dorder) values(13, 'Start Send Date', 13);
insert into page_columns (id, name, dorder) values(14, 'End Send Date', 14);
insert into page_columns (id, name, dorder) values(15, '# images sent', 15);
insert into page_columns (id, name, dorder) values(16, 'PRIMAL ID', 16);
insert into page_columns (id, name, dorder) values(17, 'Sender AET', 17);
insert into page_columns (id, name, dorder) values(18, 'Case ID', 18);
insert into locker (pproc, plock) values ('config_edit', '0');
create index image_puid_indx on image(puid);
create index series_modality on series(Modality);

