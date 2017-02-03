*********************************************************************
*     Dump weights
*     Created: 8-Jan-1999 Harrison B. Prosper
*********************************************************************
      subroutine jmOpenWeights(filename)
      character*(*) filename
      integer lun
      parameter ( lun = 99 )
      open (unit=lun,file=filename(1:len(filename)),status='UNKNOWN',
     * form='FORMATTED')
      return
      end
*********************************************************************
      subroutine jmDumpWeights()
      integer lun
      parameter ( lun = 99 )
      call jmdump(lun)
      write(lun,'(A)') '\nINPUT VARIABLES'
      return
      end
****************************
      subroutine jmReadWeights(filename,status)
      character*(*) filename
      integer status
      integer lun
      parameter ( lun = 99 )
****************************
      open (unit=lun,file=filename(1:len(filename)),status='OLD',
     * form='FORMATTED',err=100)
      call jmread(lun)
      close(unit=lun)
      status = 0
      return
 100  continue
      status =-1
      return
      end
***********************************************************************
*     Dump JNDAT1
***********************************************************************
      Subroutine jmDumpParams()
      integer maxi, maxo
      PARAMETER(MAXI=50000,MAXO=1000)
      COMMON /JNDAT1/ MSTJN(40),PARJN(40),MSTJM(20),PARJM(20),
     &                OIN(MAXI),OUT(MAXO),MXNDJM
***********************************************************************     
      print*,' '
      print*,'FORTRAN COMMON BLOCKS'
      do i = 1, 20, 2
         write(6,
     *       '(''MSTJM['',i2,'']='',I4,  '' MSTJM['',i2,'']='',I4,'//
     *       ''' PARJM['',i2,'']='',F8.3,'' PARJM['',i2,'']='',F8.3)') 
     *        i, mstjm(i), i+1, mstjm(i+1), i, parjm(i), i+1, parjm(i+1)
      enddo
      print*,' '
c
      return
      END

      Subroutine jmGetWeights(INOD, wt)
      integer INOD
      real wt(*)
      integer lun
      parameter ( lun = 99 )
******************************************
      PARAMETER(MAXV=2000,MAXM=150000,MAXI=50000,MAXO=1000,MAXD=2)

      COMMON /JNDAT1/ MSTJN(40),PARJN(40),MSTJM(20),PARJM(20),
     &                OIN(MAXI),OUT(MAXO),MXNDJM
      COMMON /JNINT1/ O(MAXV),A(MAXV),D(MAXV),T(MAXV),DT(MAXV),
     &                W(MAXM),DW(MAXM),NSELF(MAXM),NTSELF(MAXV),
     &                G(MAXM+MAXV),ODW(MAXM),ODT(MAXV),ETAV(MAXM+MAXV)
      COMMON /JMINT1/ NDIM,ISW(10),NODES(0:MAXD+1),NBO
      SAVE /JNDAT1/,/JNINT1/,/JMINT1/

      DIMENSION INDW(MAXV)
      EQUIVALENCE (NTSELF(1),INDW(1))
         
      IW=INDW(INOD)-1
      DO 200 K=1, NODES(0)
         wt(K) = W(IW+K)
 200  CONTINUE

      RETURN
      END

      Subroutine jmWriteName(string, mean, sigma)
      character*(*) string
      real mean, sigma
      integer lun
      parameter ( lun = 99 )
      write(Lun,'(A48,2(1x,F10.2))') string(1:len(string)), mean, sigma
      RETURN
      END
      
      Subroutine jmCloseWeights()
      integer lun
      parameter ( lun = 99 )
      close(unit=lun)
      RETURN
      END









