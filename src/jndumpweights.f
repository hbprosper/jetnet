*********************************************************************
*     Dump weights
*     Created: 8-Jan-1999 Harrison B. Prosper
*********************************************************************
      subroutine jnDumpWeights(filename)
      character*(*) filename
      integer lun
      parameter ( lun = 99 )
****************************
      open (unit=lun,file=filename(1:len(filename)),status='UNKNOWN',
     * form='FORMATTED')
      call jndump(lun)
      close(unit=lun)
      return
      end
****************************
      subroutine jnReadWeights(filename,status)
      character*(*) filename
      integer status
      integer lun
      parameter ( lun = 99 )
****************************
      open (unit=lun,file=filename(1:len(filename)),status='OLD',
     * form='FORMATTED',err=100)
      call jnread(lun)
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
      Subroutine jnDumpParams
      integer maxi, maxo
      PARAMETER(MAXI=50000,MAXO=1000)
      COMMON /JNDAT1/ MSTJN(40),PARJN(40),MSTJM(20),PARJM(20),
     &                OIN(MAXI),OUT(MAXO),MXNDJM
***********************************************************************     
      print*,'(FORTRAN)'
      print*,' '
      do i = 1, 40, 2
         write(6,
     *       '(''MSTJN['',i2,'']='',I4,  '' MSTJN['',i2,'']='',I4,'//
     *       ''' PARJN['',i2,'']='',F8.3,'' PARJN['',i2,'']='',F8.3)') 
     *        i, mstjn(i), i+1, mstjn(i+1), i, parjn(i), i+1, parjn(i+1)
      enddo
      print*,' '
c
      return
      END

      Subroutine jnDumpWeightsMLP(string)
      character*(*) string
      integer lun
      parameter ( lun = 99 )
******************************************
C...Dumps weights in MLPfit format

      PARAMETER(MAXV=2000,MAXM=150000,MAXI=50000,MAXO=1000)
      COMMON /JNINT1/ O(MAXV),A(MAXV),D(MAXV),T(MAXV),DT(MAXV),
     &                W(MAXM),DW(MAXM),NSELF(MAXM),NTSELF(MAXV),
     &                G(MAXM+MAXV),ODW(MAXM),ODT(MAXV),ETAV(MAXM+MAXV)
      COMMON /JNINT2/ M(0:10),MV0(11),MM0(11),NG(10),NL,IPOTT,
     &                ER1,ER2,SM(10),ICPON
      Integer N, L, I, J, NP
******************************************

      open (unit=lun,file=string(1:len(string)),status='UNKNOWN',
     *     form='FORMATTED')

C     Count number of parameters
      NP = 0
      DO L = 1, NL
         NP = NP + M(L)*(M(L-1)+1)
      EndDo

      WRITE(LUN,'('' JETNET V3.4 weights '',5I5)') (M(L), L = 0, NL)
      WRITE(LUN,*) NP

      n = 0
C     Loop over Layers
      DO L = 1, NL

C     Loop over hidden (or output) layer
         DO I = 1, M(L)

C     Write out threshold for ith output of hidden layer
            
            n = n + 1
            write(LUN,*) T(JNINDX(L,I,0))
 
            do j = 1, M(L-1)
               
C     Write out weights for jth input of ith output of hidden layer
               n = n + 1
               write(Lun,*) W(JNINDX(L,I,J))
            Enddo
         Enddo
      Enddo
      write(Lun,'(A)') 'Inputs'
      RETURN
      END

      Subroutine jnWriteName(string, mean, sigma)
      character*(*) string
      real mean, sigma
      integer lun
      parameter ( lun = 99 )
      write(Lun,'(A,2(1x,F10.2))') string(1:len(string)), mean, sigma
      RETURN
      END
      
      Subroutine jnCloseWeights(out)
      integer out
      integer lun
      parameter ( lun = 99 )
      if ( out .eq. 0 ) then
         write(Lun,'(A)') 'Sigmoid Output'
      else
         write(Lun,'(A)') 'Linear Output'
      endif
      close(unit=lun)
      RETURN
      END









