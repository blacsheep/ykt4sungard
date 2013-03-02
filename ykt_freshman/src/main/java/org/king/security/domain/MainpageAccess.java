package org.king.security.domain;
// Generated by MyEclipse - Hibernate Tools

import java.sql.Date;
import java.sql.Time;


/**
 * MainpageAccess generated by MyEclipse - Hibernate Tools
 */
public class MainpageAccess extends AbstractMainpageAccess implements java.io.Serializable {

    // Constructors

    /** default constructor */
    public MainpageAccess() {
    }

	/** minimal constructor */
    public MainpageAccess(String userName, Date accessDate, Time accessTime) {
        super(userName, accessDate, accessTime);        
    }
    
    /** full constructor */
    public MainpageAccess(String userName, String trueName, Date accessDate, Time accessTime, String ipAddress) {
        super(userName, trueName, accessDate, accessTime, ipAddress);        
    }
   
}