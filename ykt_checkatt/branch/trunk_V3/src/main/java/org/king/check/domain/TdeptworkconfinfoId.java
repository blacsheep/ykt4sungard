package org.king.check.domain;



/**
 * TdeptworkconfinfoId generated by MyEclipse - Hibernate Tools
 */

public class TdeptworkconfinfoId  implements java.io.Serializable {


    // Fields    

     private String deptId;
     private String begindate;


    // Constructors

    /** default constructor */
    public TdeptworkconfinfoId() {
    }

    

   
    // Property accessors

    public String getDeptId() {
        return this.deptId;
    }
    
    public void setDeptId(String deptId) {
        this.deptId = deptId;
    }

    public String getBegindate() {
        return this.begindate;
    }
    
    public void setBegindate(String begindate) {
        this.begindate = begindate;
    }
   



   public boolean equals(Object other) {
         if ( (this == other ) ) return true;
		 if ( (other == null ) ) return false;
		 if ( !(other instanceof TdeptworkconfinfoId) ) return false;
		 TdeptworkconfinfoId castOther = ( TdeptworkconfinfoId ) other; 
         
		 return ( (this.getDeptId()==castOther.getDeptId()) || ( this.getDeptId()!=null && castOther.getDeptId()!=null && this.getDeptId().equals(castOther.getDeptId()) ) )
 && ( (this.getBegindate()==castOther.getBegindate()) || ( this.getBegindate()!=null && castOther.getBegindate()!=null && this.getBegindate().equals(castOther.getBegindate()) ) );
   }
   
   public int hashCode() {
         int result = 17;
         
         result = 37 * result + ( getDeptId() == null ? 0 : this.getDeptId().hashCode() );
         result = 37 * result + ( getBegindate() == null ? 0 : this.getBegindate().hashCode() );
         return result;
   }   





}