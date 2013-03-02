-- Start of generated script for 10.108.0.222-DB2-YKT (db2inst4)
--  May-16-2007 at 09:19:53

DROP TABLE YKT_CONF.T_ASSEMBLY_ROOM;

DROP TABLE YKT_CONF.T_ATTENDEE_GROUP;

DROP TABLE YKT_CONF.T_ATTENDEE_LIST;

DROP TABLE YKT_CONF.T_CONFERENCE;

DROP TABLE YKT_CONF.T_CONFERENCE_TYPE;

DROP TABLE YKT_CONF.T_DELEGATE;

DROP TABLE YKT_CONF.T_DEPT_LIMIT;

DROP TABLE YKT_CONF.T_DEV_CONFERENCE;

DROP TABLE YKT_CONF.T_DUTY;

DROP TABLE YKT_CONF.T_FUNC_LIST;

DROP TABLE YKT_CONF.T_GROUP;

DROP TABLE YKT_CONF.T_INFORM_LIST;

DROP TABLE YKT_CONF.T_OPERATOR;

DROP TABLE YKT_CONF.T_OPER_LIMIT;

DROP TABLE YKT_CONF.T_RECORDHIS;

CREATE TABLE YKT_CONF.T_ASSEMBLY_ROOM
 (ROOM_ID    VARCHAR(8)      NOT NULL,
  ROOM_NAME  VARCHAR(80),
  ADDRESS    VARCHAR(120),
  SIGN       CHARACTER(1),
  CONTAIN    INTEGER,
  COMMENTS   VARCHAR(200),
  PRIMARY KEY(ROOM_ID)
 );
 


CREATE TABLE YKT_CONF.T_ATTENDEE_GROUP
 (CUST_ID  INTEGER         NOT NULL,
  GROUP_ID INTEGER         NOT NULL,
  COMMENTS VARCHAR(120),
  PRIMARY KEY
   (CUST_ID,
    GROUP_ID
   )
 );


CREATE TABLE YKT_CONF.T_ATTENDEE_LIST
 (CON_ID             INTEGER         NOT NULL,
  CUST_ID            INTEGER         NOT NULL,
  CARD_NO            INTEGER,
  ALLOT_DATE         CHARACTER(8),
  ALLOT_TIME         VARCHAR(6),
  SEND_SIGN          CHARACTER(1),
  DEL_SIGN           CHARACTER(1),
  ATTENDEE_TYPE      CHARACTER(1),
  ATTEND_SIGN        CHARACTER(2),
  ATTEND_DATE        CHARACTER(8),
  ATTEND_TIME        VARCHAR(6),
  LEAVE_REASON       VARCHAR(300),
  REPLACER_ID        VARCHAR(10),
  REPLACER_NAME      VARCHAR(60),
  REPLACER_DEPT      VARCHAR(120),
  REPLACER_DUTY      VARCHAR(120),
  REPLACER_COMMENTS  VARCHAR(300),
  DELEGRAY           INTEGER,
  PRIMARY KEY
   (CON_ID,
    CUST_ID
   )
 );
 

CREATE TABLE YKT_CONF.T_CONFERENCE
 (CON_ID         INTEGER         NOT NULL,
  CON_NAME       VARCHAR(120),
  ROOM_ID        VARCHAR(8),
  TYPE_NAME      VARCHAR(90),
  CON_BEGINDATE  CHARACTER(8),
  CON_SIGNTIME   VARCHAR(6),
  CON_BEGINTIME  VARCHAR(6),
  CON_ENDDATE    CHARACTER(8),
  CON_ENDTIME    VARCHAR(6),
  ORGANIGER_ID   VARCHAR(10),
  COMPERE        VARCHAR(20),
  CON_EXPLAIN    VARCHAR(300),
  IF_SERECY      CHARACTER(1),
  STATUS         CHARACTER(1)              DEFAULT '0',
  CONTENT        VARCHAR(3000),
  COMMENTS       VARCHAR(100),
  PREPLAN_1      VARCHAR(50),
  PREPLAN_2      VARCHAR(50),
  PREPLAN_3      VARCHAR(50),
  PRIMARY KEY
   (CON_ID
   )
 );


CREATE TABLE YKT_CONF.T_CONFERENCE_TYPE
 (TYPE_ID    INTEGER         NOT NULL,
  TYPE_NAME  VARCHAR(90),
  COMMENTS   VARCHAR(240),
  PRIMARY KEY
   (TYPE_ID
   )
 );
 

CREATE TABLE YKT_CONF.T_DELEGATE
 (DLGT_ID    INTEGER         NOT NULL,
  DLGT_NAME  VARCHAR(120),
  DEPT_ID    VARCHAR(20),
  "OPEN"       CHARACTER(1),
  OPER_ID    VARCHAR(20)     NOT NULL,
  "COMMENT"    VARCHAR(120),
  PRIMARY KEY
   (DLGT_ID
   )
 );
 

CREATE TABLE YKT_CONF.T_DEPT_LIMIT
 (OPER_CODE  VARCHAR(20)     NOT NULL,
  DEPT_ID    VARCHAR(10)     NOT NULL,
  PRIMARY KEY
   (OPER_CODE,
    DEPT_ID
   )
 );
  

CREATE TABLE YKT_CONF.T_DEV_CONFERENCE
 (CON_ID         INTEGER         NOT NULL,
  DEVICE_ID      INTEGER         NOT NULL,
  CON_BEGINDATE  CHARACTER(8),
  CON_BEGINTIME  VARCHAR(6),
  CON_ENDTIME    VARCHAR(6),
  CON_SIGNTIME   VARCHAR(6),
  PRIMARY KEY
   (CON_ID,
    DEVICE_ID
   )
 );
  

CREATE TABLE YKT_CONF.T_DUTY
 (STUEMP_NO  VARCHAR(20)     NOT NULL,
  OA_ACCOUNT VARCHAR(50)     NOT NULL,
  DUTY       VARCHAR(300),
  PRIMARY KEY
   (STUEMP_NO,
    OA_ACCOUNT
   )
 );
 

CREATE TABLE YKT_CONF.T_FUNC_LIST
 (FUNC_CODE  VARCHAR(4)      NOT NULL,
  FUNC_NAME  VARCHAR(200),
  FUNC_URL   VARCHAR(400),
  PRIMARY KEY
   (FUNC_CODE
   )
 );
  

CREATE TABLE YKT_CONF.T_GROUP
 (GROUP_ID    INTEGER         NOT NULL,
  OPER_ID     VARCHAR(20)     NOT NULL,
  GROUP_NAME  VARCHAR(120),
  COMMENTS    VARCHAR(120),
  PRIMARY KEY
   (GROUP_ID
   )
 );
 

CREATE TABLE YKT_CONF.T_INFORM_LIST
 (CUST_ID      INTEGER         NOT NULL,
  CON_ID       INTEGER         NOT NULL,
  INFORM_DATE  CHARACTER(8),
  INFORM_TIME  CHARACTER(6),
  INFORM_SIGN  CHARACTER(1),
  PRIMARY KEY
   (CUST_ID,
    CON_ID
   )
 );
 

CREATE TABLE YKT_CONF.T_OPERATOR
 (OPER_CODE  VARCHAR(20)     NOT NULL,
  OPER_NAME  VARCHAR(60),
  OPER_PWD   VARCHAR(100),
  DEPT_ID    VARCHAR(10),
  LOGIN_TIME VARCHAR(20),
  LOGIN_IP   VARCHAR(15),
  STATUS     CHARACTER(1),
  PRIMARY KEY
   (OPER_CODE
   )
 );
 

CREATE TABLE YKT_CONF.T_OPER_LIMIT
 (OPER_CODE  VARCHAR(20)     NOT NULL,
  FUNC_CODE  VARCHAR(4)      NOT NULL,
  PRIMARY KEY
   (OPER_CODE,
    FUNC_CODE
   )
 );
 

CREATE TABLE YKT_CONF.T_RECORDHIS
 (ID           INTEGER         NOT NULL ,
  CONFID       INTEGER,
  PHYSICAL_NO  VARCHAR(40),
  CHECKTIME    VARCHAR(16),
  ATTEND_SIGN  CHARACTER(2),
  DEALORNOT    CHARACTER(2),
  PRIMARY KEY
   (ID
   )
 );




-- End of generated script for 10.108.0.222-DB2-YKT (db2inst4)